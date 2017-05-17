#include "ofApp.h"
#include "stb_image.h"

#define TEST_IMAGE_GENERATE 0


static const int kTipCount = 1000;

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetVerticalSync(false);

#if TEST_IMAGE_GENERATE
	ofTrueTypeFont font;
	font.load("frabk.ttf", 20);
	ofFbo fbo;
	fbo.allocate(100, 120);
	ofPixels pixels;

	ofVec3f colors[] = {
		{ 34, 0, 77 },
		{ 80, 0, 255 },
		{ 0, 204, 128 },
		{ 0, 255, 0 },
		{ 255, 72, 0 },
		{ 224, 0, 0 },
		{ 52, 0, 0 },
		{ 34, 0, 77 },
	};
	for (int i = 0; i < kTipCount; ++i) {
		fbo.begin();
		ofClear(0, 255);

		// 
		auto color = colors[i % 7];
		ofSetColor(color.x, color.y, color.z);
		ofDrawRectangle(0, 0, fbo.getWidth(), fbo.getHeight());

		color *= 0.8;
		ofSetColor(color.x, color.y, color.z);
		ofDrawRectangle(fbo.getWidth() * 0.5, 0, fbo.getWidth() * 0.5, fbo.getHeight() * 0.5);
		ofDrawRectangle(0, fbo.getHeight() * 0.5, fbo.getWidth() * 0.5, fbo.getHeight() * 0.5);

		char text[128];
		sprintf(text, "%d", i);
		ofRectangle r = font.getStringBoundingBox(text, 0, 0);
		ofPushMatrix();
		ofTranslate(ofPoint(fbo.getWidth(), fbo.getHeight()) * 0.5f -  r.getCenter());
		ofSetColor(255);
		font.drawString(text, 0, 0);
		ofPopMatrix();
		fbo.end();

		fbo.readToPixels(pixels);
		sprintf(text, "testimage\\image_%05d.png", i);
		ofSaveImage(pixels, text);
	}
#endif

	_imgui.setup();

	_camera.setNearClip(0.1f);
	_camera.setFarClip(500.0f);
	_camera.setDistance(5.0f);

	_renderer = new TexturedQuadRenderer(kTipCount);

	#pragma omp parallel for schedule(dynamic, 100)
	for (int i = 0; i < kTipCount; ++i) {
		char text[256];
		sprintf(text, "testimage\\image_%05d.png", i);
		int x,y,n;
		unsigned char *data = stbi_load(ofToDataPath(text).c_str(), &x, &y, &n, STBI_rgb_alpha);

		TexturedQuadRenderer::TipType tip;
		if (x != tip.width() || y != tip.height()) { abort(); }

		memcpy(tip.data(), data, x * y * 4);
		stbi_image_free(data);
		
		_renderer->setTip(tip, i);
	}
	_renderer->updateTip();
	_renderer->writeBufferImage();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){

	ofEnableDepthTest();



	ofClear(0);

	_camera.begin();
	ofPushMatrix();
	ofRotateZ(90.0f);
	ofSetColor(64);
	ofDrawGridPlane(1, 10);
	ofPopMatrix();

	ofPushMatrix();
	ofDrawAxis(50);
	ofPopMatrix();

	//TexturedQuadRenderer::Instance *instances = _renderer->map(1);
	//{
	//	for (int j = 0; j < 3; ++j) {
	//		instances[0].position[j] = 0.0f;
	//	}
	//	instances[0].scale = 1.0f;
	//	instances[0].tipinfo = _renderer->tipinfo(1);
	//}
	//_renderer->unmap();

	float e = ofGetElapsedTimef();
	int N = 100000;

	TexturedQuadRenderer::Instance *instances = _renderer->map(N);
	for (int i = 0; i < N; ++i) {
		float x = ofNoise(i * 0.01f, 0.0f, e * 0.1f) * 2.0f - 1.0f;
		float y = ofNoise(0.0f, i * 0.01f, e * 0.1f) * 2.0f - 1.0f;
		float z = ofNoise(0.0f, e * 0.1f, i * 0.01f) * 2.0f - 1.0f;

		ofVec3f p = ofVec3f(x, y, z) * 40.0f;
		for (int j = 0; j < 3; ++j) {
			instances[i].position[j] = p[j];
		}
		instances[i].scale = 0.5f;
		instances[i].tipinfo = _renderer->tipinfo(i % kTipCount);
	}
	_renderer->unmap();

	_renderer->draw();


	_camera.end();


	ofDisableDepthTest();
	ofSetColor(255);


	_imgui.begin();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ofVec4f(0.2f, 0.2f, 0.5f, 0.5f));
	ImGui::SetNextWindowPos(ofVec2f(500, 30), ImGuiSetCond_Once);
	ImGui::SetNextWindowSize(ofVec2f(500, 600), ImGuiSetCond_Once);

	ImGui::Begin("Config Panel");
	ImGui::Text("fps: %.2f", ofGetFrameRate());

	auto wp = ImGui::GetWindowPos();
	auto ws = ImGui::GetWindowSize();
	ofRectangle win(wp.x, wp.y, ws.x, ws.y);

	ImGui::End();
	ImGui::PopStyleColor();

	_imgui.end();

	if (win.inside(ofGetMouseX(), ofGetMouseY())) {
		_camera.disableMouseInput();
	}
	else {
		_camera.enableMouseInput();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == 'f') {
		ofToggleFullscreen();
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
