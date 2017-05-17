#pragma once

/*
	render = new render()
	render->setTile(.., 0);

	render->map();

	for(;;) {
		render->setTileIndex(data, 10);
		data.x = 0;
		data.y = 0;
	}
*/
/*
RGBA Image Tip
+-------> x
|
|
y
*/
template <int W, int H>
class ImageTip {
public:
	int width() const {
		return W;
	}
	int height() const {
		return H;
	}

	uint8_t *data() {
		return _data;
	}
	const uint8_t *data() const {
		return _data;
	}

#ifdef OF_VERSION_MAJOR
	void load(ofPixels &image) {
		if (image.getWidth() != W || image.getHeight() != H) {
			image.resize(W, H, OF_INTERPOLATE_BICUBIC);
			printf("warning: ImageTip resize image %dx%d to %dx%d\n", 
				(int)image.getWidth(), (int)image.getHeight(),
				W, H);
		}
		image.setImageType(OF_IMAGE_COLOR_ALPHA);
		memcpy(_data, image.getData(), W * H * 4);
	}
#endif

	uint8_t _data[W * H * 4];
};

template <int TipW, int TipH>
class TextureArrayBuffer {
public:
	using TipType = ImageTip<TipW, TipH>;

	TextureArrayBuffer(int width, int height, int tipCountHint)
		: _width(width)
		, _height(height) {

		_columns = width / TipW;
		_rows = height / TipH;

		_layerCount = (tipCountHint / (_columns * _rows)) + 1;
		_data.resize(_layerCount * width * height * 4, 255);

		_tipCount = _columns * _rows * _layerCount;
		_tipinfos.resize(_tipCount);
	}

	struct TipInfo {
		float uv[2] = { 0.0f, 0.0f };
		float layer = 0.0f;
	};

	TipInfo tipinfo(int index) const {
		return _tipinfos[index];
	}
	void setTip(const TipType &tip, int index) {
		if (_tipCount <= index) {
			abort();
		}

		// チップ座標計算
		TipInfo info;
		int layer = index / (_columns * _rows);
		info.layer = layer;

		int index_on_layer = index % (_columns * _rows);
		int r = index_on_layer / _columns;
		int c = index_on_layer % _columns;

		float stepU = 1.0f / _width;
		float stepV = 1.0f / _height;
		float stepTipU = stepU * TipW;
		float stepTipV = stepV * TipH;

		info.uv[0] = stepTipU * c;
		info.uv[1] = stepTipV * r;
		_tipinfos[index] = info;

		// copy
		int dst_orig_x = TipW * c;
		int dst_orig_y = TipH * r;

		int layer_orig = layer * _width * _height * 4;

		uint8_t *dst = _data.data() + layer_orig;
		const uint8_t *src = tip.data();
		for (int y = 0; y < tip.height(); ++y) {
			int dst_y = dst_orig_y + y;
			int dst_ybase = dst_y * _width;
			int src_ybase = tip.width() * (tip.height() - y - 1);
			for (int x = 0; x < tip.width(); ++x) {
				int dst_x = dst_orig_x + x;
				int src_index = (src_ybase + x) * 4;
				int dst_index = (dst_ybase + dst_x) * 4;

				for (int i = 0; i < 4; ++i) {
					dst[dst_index + i] = src[src_index + i];
				}
			}
		}
	}

	int _width = 0;
	int _height = 0;
	int _layerCount = 0;

	/*
	Array Texture Buffer

	v
	|
	|
	+-------> u
	*/
	std::vector<uint8_t> _data;
	int _columns = 0;
	int _rows = 0;
	int _tipCount = 0;
	std::vector<TipInfo> _tipinfos;

	float tipstepU() const {
		float uwhole = float(_columns * TipW) / _width;
		return uwhole / _columns;
	}
	float tipstepV() const {
		float vwhole = float(_rows * TipH) / _height;
		return vwhole / _rows;
	}

#ifdef OF_VERSION_MAJOR
	void writeBufferImage() {
		for (int i = 0; i < _layerCount; ++i) {
			int offset = _width * _height * 4 * i;
			ofImage image;
			image.setFromPixels(_data.data() + offset, _width, _height, OF_IMAGE_COLOR_ALPHA);
			char name[256];
			sprintf(name, "arraybufferimage/array_buffer_image_%02d.png", i);
			image.save(name);
		}
	}
#endif
};

class TexturedQuadRenderer {
public:
	struct Vertex {
		Vertex() {
		}
		Vertex(float x, float y, float z, float u, float v, float nx, float ny, float nz) {
			position[0] = x;
			position[1] = y;
			position[2] = z;

			texcoord[0] = u;
			texcoord[1] = v;

			normal[0] = nx;
			normal[1] = ny;
			normal[2] = nz;
		}
		float position[3] = {};
		float texcoord[2] = {};
		float normal[3] = {};
	};

	// Fixed
	enum {
		TipW = 100,
		TipH = 120,
	};
	using TextureArrayBufferType = TextureArrayBuffer<TipW, TipH>;
	using TipType = TextureArrayBufferType::TipType;
	struct Instance {
		float position[3] = {};
		float scale = 1.0f;
		TextureArrayBufferType::TipInfo tipinfo;
	};
	ofBufferObject _vertexBuffer;
	ofBufferObject _indexBuffer;
	ofBufferObject _instanceBuffer;
	ofShader _shader;
	int _instanceCap = 10;
	int _instanceCount = 0;
	GLuint _vao = 0;

	GLuint _texture;
	std::unique_ptr<TextureArrayBufferType> _textureBuffer;

	TexturedQuadRenderer(int tipCountHint) {
		_textureBuffer = std::unique_ptr<TextureArrayBufferType>(new TextureArrayBufferType(4096, 4096, tipCountHint));

		glGenVertexArrays(1, &_vao);

		// TODO 幅が1のほうがいいかも

		// 左上から時計回り
		float size = 0.5f;
		//std::vector<Vertex> vertices = {
		//	Vertex{ -size, size, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
		//	Vertex{ size, size, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f },
		//	Vertex{ size, -size, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		//	Vertex{ -size, -size, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
		//};
		std::vector<Vertex> vertices = {
			Vertex{ -size, size, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f },
			Vertex{ size, size, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
			Vertex{ size, -size, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f },
			Vertex{ -size, -size, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f },
		};
		_vertexBuffer.allocate(vertices, GL_STATIC_DRAW);

		std::vector<uint16_t> indices = {
			1, 0, 3,
			1, 3, 2
		};
		_indexBuffer.allocate<uint16_t>(indices, GL_STATIC_DRAW);
		_instanceBuffer.allocate(sizeof(Instance) * _instanceCap, GL_STREAM_DRAW);

		_shader.load("textured_instanced_vert.glsl", "textured_instanced_frag.glsl");

		glGenTextures(1, &_texture);
		glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, _texture);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_RGBA, _textureBuffer->_width /*w*/, _textureBuffer->_height /*h*/, _textureBuffer->_layerCount /*d*/, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);
	}
	~TexturedQuadRenderer() {
		glDeleteVertexArrays(1, &_vao);
		glDeleteTextures(1, &_texture);
	}

	void updateTip() {
		glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, _texture);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, 0, 0, 0, _textureBuffer->_width /*w*/, _textureBuffer->_height /*h*/, _textureBuffer->_layerCount /*d*/, GL_RGBA, GL_UNSIGNED_BYTE, _textureBuffer->_data.data());
		glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);
	}

	TextureArrayBufferType::TipInfo tipinfo(int index) const {
		return _textureBuffer->tipinfo(index);
	}
	void setTip(const TextureArrayBufferType::TipType &tip, int index) {
		_textureBuffer->setTip(tip, index);
	}

	void reserve(int instanceCount) {
		if (_instanceCap <= instanceCount) {
			_instanceCap = instanceCount;
			_instanceBuffer.allocate(sizeof(Instance) * _instanceCap, GL_STREAM_DRAW);
		}
	}
	Instance *map(int instanceCount) {
		reserve(instanceCount);
		_instanceCount = instanceCount;
		return (Instance *)_instanceBuffer.map(GL_WRITE_ONLY);
	}
	void unmap() {
		_instanceBuffer.unmap();
	}

	void draw() {
		if (_instanceCount <= 0) {
			return;
		}
		glBindVertexArray(_vao);

		auto modelview = ofGetCurrentMatrix(OF_MATRIX_MODELVIEW);
		auto proj = ofGetCurrentMatrix(OF_MATRIX_PROJECTION);

		int a_position = _shader.getAttributeLocation("a_position");
		int a_texcoord = _shader.getAttributeLocation("a_texcoord");
		int a_normal = _shader.getAttributeLocation("a_normal");
		int a_location = _shader.getAttributeLocation("a_location");
		int a_scale = _shader.getAttributeLocation("a_scale");
		int a_uv = _shader.getAttributeLocation("a_uv");
		int a_layer = _shader.getAttributeLocation("a_layer");

		_vertexBuffer.bind(GL_ARRAY_BUFFER);
		glEnableVertexAttribArray(a_position);
		glVertexAttribPointer(a_position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char *)0 + offsetof(Vertex, position));
		glEnableVertexAttribArray(a_texcoord);
		glVertexAttribPointer(a_texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char *)0 + offsetof(Vertex, texcoord));
		glEnableVertexAttribArray(a_normal);
		glVertexAttribPointer(a_normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (char *)0 + offsetof(Vertex, normal));
		_vertexBuffer.unbind(GL_ARRAY_BUFFER);

		_instanceBuffer.bind(GL_ARRAY_BUFFER);
		glEnableVertexAttribArray(a_location);
		glVertexAttribPointer(a_location, 3, GL_FLOAT, GL_FALSE, sizeof(Instance), (char *)0 + offsetof(Instance, position));
		glVertexAttribDivisor(a_location, 1);

		glEnableVertexAttribArray(a_scale);
		glVertexAttribPointer(a_scale, 1, GL_FLOAT, GL_FALSE, sizeof(Instance), (char *)0 + offsetof(Instance, scale));
		glVertexAttribDivisor(a_scale, 1);

		glEnableVertexAttribArray(a_uv);
		glVertexAttribPointer(a_uv, 2, GL_FLOAT, GL_FALSE, sizeof(Instance), (char *)0 + offsetof(Instance, tipinfo.uv));
		glVertexAttribDivisor(a_uv, 1);

		glEnableVertexAttribArray(a_layer);
		glVertexAttribPointer(a_layer, 1, GL_FLOAT, GL_FALSE, sizeof(Instance), (char *)0 + offsetof(Instance, tipinfo.layer));
		glVertexAttribDivisor(a_layer, 1);

		_instanceBuffer.unbind(GL_ARRAY_BUFFER);

		_indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
		_shader.begin();
		_shader.setUniformMatrix4f("u_modelview", modelview);
		_shader.setUniformMatrix4f("u_proj", proj);
		
		_shader.setUniform2f("u_uvtipstep", _textureBuffer->tipstepU(), _textureBuffer->tipstepV());
		_shader.setUniform1f("u_aspect", float(TipW) / float(TipH));

		_shader.setUniform1i("u_texture", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, _texture);
		//
		//static bool bang = true;
		//static ofImage image;
		//if (bang) {
		//	bang = false;
		//	ofDisableArbTex();
		//	image.load("mini_lena.png");
		//	ofEnableArbTex();
		//}
		//_shader.setUniformTexture("u_texture", image.getTexture(), 1);

		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0, _instanceCount);

		_shader.end();
		_indexBuffer.unbind(GL_ELEMENT_ARRAY_BUFFER);

		glDisableVertexAttribArray(a_position);
		glDisableVertexAttribArray(a_texcoord);
		glDisableVertexAttribArray(a_normal);
		glDisableVertexAttribArray(a_location);
		glDisableVertexAttribArray(a_scale);
		glDisableVertexAttribArray(a_uv);
		glDisableVertexAttribArray(a_layer);

		glBindVertexArray(0);
	}

#ifdef OF_VERSION_MAJOR
	void writeBufferImage() {
		_textureBuffer->writeBufferImage();
	}
#endif

};
