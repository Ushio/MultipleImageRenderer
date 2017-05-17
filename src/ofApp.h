#pragma once

#include "ofMain.h"
#include "ofxImGui.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "texturedquadrenderer.hpp"

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



class ofApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	ofEasyCam _camera;
	ofxImGui::Gui _imgui;

	TextureArrayBuffer<128, 128> *_textureArrayBuffer = nullptr;
	TexturedQuadRenderer *_renderer = nullptr;
};
