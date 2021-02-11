#ifdef _MSC_VER
#include <Windows.h>
#endif
#include "ImgviewApp.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <boost/locale.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

#ifdef _MSC_VER
std::wstring ToUtf16(std::string str)
{
	std::wstring ret;
	int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
	if (len > 0) {
		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &ret[0], len);
	}
	return ret;
}
#endif


const std::vector<std::string> supportedExtensions = {
	".png", ".jpg", ".jpeg",// ".gif",
	".PNG", ".JPG", ".JPEG",//, ".GIF"
	".webp", ".WEBP",
	".tif", ".tiff", ".TIF", ".TIFF",
	".pgm", ".PGM"
	//".exr", ".EXR"
};

const int initWidth = 1000, initHeight = 1000;
float offset[] = {0.f, -initHeight};
const float multisampleFactor = 2.f;
const float minZoom = 0.01f;
#ifdef __APPLE__
const float scrollSpeed = 0.01f;
#else
const float scrollSpeed = 0.1f;
#endif

GLuint compileShader(const std::string &source, GLenum type);
GLuint createShaderProgram(GLuint vertShader, GLuint fragShader);
void setupVertexBuffers(GLuint program);

ImgviewApp::ImgviewApp(const std::string &filename)
	:redraw_(true),
	running_(true),
	mouseDown_(false),
	zoom_(1.0f),
	winAspect_((float)(initWidth) / (float)(initHeight)),
	winWidth_(initWidth), winHeight_(initHeight),
	isFullscreen_(false),
	context_(0, 30, initWidth, initHeight),
	samplingMode_(SamplingMode::LINEAR)
{
	glViewport(0, 0, initWidth, initHeight);
    // Create and install global locale
	boost::locale::generator gen;
	gen.use_ansi_encoding(true);
    std::locale::global(gen.generate(""));
    // Make boost.filesystem use it
    boost::filesystem::path::imbue(std::locale());
    // Now Works perfectly fine with UTF-8!

	initialImagePath_ = filename;
	if(!boost::filesystem::is_regular_file(initialImagePath_)) {
		throw std::runtime_error(
			"Supplied image is not present, or not a regular file");
	}

	loadImagePaths();
	loadImage(filename);
	fitImageToWindow();
	
	setTitle();
}

ImgviewApp::~ImgviewApp() throw()
{
	if(texture_ != 0) {
		glDeleteTextures(1, &texture_);
	}
}

void ImgviewApp::run()
{
	SDL_Event event;

	while(running_) {

		if(redraw_) {
			redrawImage();
			redraw_ = false;
		}

		while(SDL_PollEvent(&event)) {
			processEvent(event);
		}
	}
}

void ImgviewApp::loadImagePaths()
{
	imagePaths_.clear();
	boost::filesystem::path imDir = initialImagePath_.parent_path();
	boost::filesystem::directory_iterator i(imDir);
	boost::filesystem::directory_iterator endItr;
	for(; i != endItr; ++i)
	{
		if(boost::filesystem::is_regular_file(*i)) {
			if(supportedFileExtension(i->path().extension().string())) {
				imagePaths_.push_back(*i);
				if(*i == initialImagePath_) {
					imagePathIdx_ = imagePaths_.size() - 1;
				}
			}
		}
	}
	if(imagePaths_.size() == 0) {
		throw std::runtime_error("No images left in directory");
	}
}

void ImgviewApp::redrawImage()
{
	glClearColor(0.1f, 0.1f, 0.1f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_);
	shaderProgram_.use();
	shaderProgram_.bindVertexArray();
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	shaderProgram_.unbindVertexArray();
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_GL_SwapWindow(context_.window());
}

const std::string controlsString =
"Left/Right: Prev/Next Image\n"
"SHIFT+Left/Right: Prev/Next Image (maintain current zoom)\n"
"Mouse drag: Move viewpoint\n"
"Mouse wheel: Zoom in/out\n"
"ALT+ENTER: Toggle Fullscreen\n"
"F: Fit image in window\n"
"SHIFT+F: Resize window to image dimensions\n"
"C: Center image in window\n"
"0: Set zoom level to 1:1\n"
"S: Switch between linear/nearest neighbour sampling\n"
"H: Show these controls\n"
"Q: Quit application\n";

void ImgviewApp::updateSamplingMode()
{
	glBindTexture(GL_TEXTURE_2D, texture_);
	if (samplingMode_ == SamplingMode::LINEAR) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else if (samplingMode_ == SamplingMode::NEAREST) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	redraw_ = true;
}

void ImgviewApp::processEvent(SDL_Event &event)
{
	if(event.type == SDL_QUIT ||
		 (event.type == SDL_WINDOWEVENT &&
		event.window.event == SDL_WINDOWEVENT_CLOSE)) {
			 running_ = false;
		 }

	if(event.type == SDL_MOUSEBUTTONDOWN &&
		 event.button.button == 1) {
		mouseDown_ = true;
	}
	if(event.type == SDL_MOUSEBUTTONUP &&
event.button.button == 1) {
mouseDown_ = false;
	}

	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_RETURN) {
			if (event.key.keysym.mod & KMOD_ALT) {
				if (isFullscreen_) {
					SDL_SetWindowFullscreen(context_.window(), 0);
					isFullscreen_ = false;
				} else {
					SDL_SetWindowFullscreen(context_.window(), SDL_WINDOW_FULLSCREEN_DESKTOP);
					isFullscreen_ = true;
				}
			}
		}

		if (event.key.keysym.sym == SDLK_q) {
			running_ = false;
		}
		if (event.key.keysym.sym == SDLK_c) {
			centerImageInWindow();
		}
		if (event.key.keysym.sym == SDLK_0) {
			resetZoom();
		}
		if (event.key.keysym.sym == SDLK_h) {
#ifdef _MSC_VER
			MessageBox(NULL, controlsString.c_str(), "imgview Controls", MB_OK);
#endif
		}
		if (event.key.keysym.sym == SDLK_s) {
			if (samplingMode_ == SamplingMode::LINEAR) {
				samplingMode_ = SamplingMode::NEAREST;
			} else if (samplingMode_ == SamplingMode::NEAREST) {
				samplingMode_ = SamplingMode::LINEAR;
			}
			updateSamplingMode();
		}
		if (event.key.keysym.sym == SDLK_f) {
			if (!(event.key.keysym.mod & KMOD_LSHIFT)) {
				fitImageToWindow();
			} else {
				fitWindowToImage();
			}
		}

		if (event.key.keysym.sym == SDLK_RIGHT) {
			++imagePathIdx_; imagePathIdx_ %= imagePaths_.size();
		}
		if (event.key.keysym.sym == SDLK_LEFT) {
			imagePathIdx_ = imagePathIdx_ == 0 ? imagePaths_.size() - 1 : imagePathIdx_ - 1;
		}
		if (event.key.keysym.sym == SDLK_LEFT ||
			event.key.keysym.sym == SDLK_RIGHT) {
			if (!loadImage(imagePaths_[imagePathIdx_].path().string())) {
				//Couldn't load this image. Try rescanning directory.
				std::cout << "Couldn't load image "
					<< imagePaths_[imagePathIdx_].path().string()
					<< " rescanning directory..." << std::endl;
				size_t tmp = imagePathIdx_;
				loadImagePaths();
				imagePathIdx_ = tmp % imagePaths_.size();

				if (!loadImage(imagePaths_[imagePathIdx_].path().string())) {
					//After rescanning directory, still couldn't load image.
					throw std::runtime_error("Couldn't open image " +
						imagePaths_[imagePathIdx_].path().string());
				}
			}
			setTitle();
			if (!(event.key.keysym.mod & KMOD_LSHIFT)) {
				//Left shift wasn't held down
				handleResize(winWidth_, winHeight_);
				fitImageToWindow();
			} else {
				redraw_ = true;
			}
		}
	}
	if (event.type == SDL_WINDOWEVENT &&
		event.window.event == SDL_WINDOWEVENT_RESIZED) {
		handleResize(event.window.data1, event.window.data2);

	}
	if (event.type == SDL_MOUSEWHEEL) {
		if (event.wheel.y != 0) {
			float newZoom = 1.f / zoom_ + scrollSpeed * event.wheel.y;
			zoom_ = newZoom > minZoom ? 1.f / newZoom : 1.f / minZoom;
			updateZoom();
			setTitle();
			redraw_ = true;
		}
	}
	if (event.type == SDL_MOUSEMOTION) {
		if (mouseDown_) {
			offset[0] -= event.motion.xrel;
			offset[1] += event.motion.yrel;
			shaderProgram_.setOffset(offset[0], offset[1]);
			redraw_ = true;
		}
	}
}


bool ImgviewApp::loadImage(const std::string &filename)
{
	// On windows, convert to wstring as only utf16 is supported.
#ifdef _MSC_VER
	std::wstring str = ToUtf16(filename);
#else
	std::string str = filename;
#endif

	//Imread doesn't handle Unicode, so need to read into an ifstream and then use imdecode
	std::ifstream f(str, std::iostream::binary);

	// Get its size
	std::filebuf* pbuf = f.rdbuf();
	size_t size = pbuf->pubseekoff(0, f.end);
	pbuf->pubseekpos(0, f.in);
	if(size == 0 || size == -1)  {
		return false;
	}

	// Put it in a vector
	std::vector<uchar> buffer(size);
	pbuf->sgetn((char*)buffer.data(), size);

	// Decode the vector
	cv::Mat mat = cv::imdecode(buffer, cv::IMREAD_COLOR);
	if(mat.rows == 0 || mat.cols == 0) {
		return false;
	}

	//std::cout << "Loaded image " << filename << " " << mat.cols << "x"
		//<< mat.rows << ", " << mat.channels() << " channels" << std::endl;

	if(mat.channels() != 3 && mat.channels() != 4) {
		mat = cv::imread(filename);
	}
	if(mat.channels() == 3) {
		cv::cvtColor(mat, mat, cv::COLOR_BGR2BGRA);
	} 

	imAspect_ = (float)(mat.cols) / (float)(mat.rows);
	imWidth_ = mat.cols; imHeight_ = mat.rows;

	if(texture_ != 0) {
		glDeleteTextures(1, &texture_);
	}

	glGenTextures(1, &texture_);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
	if (samplingMode_ == SamplingMode::LINEAR) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	} else if (samplingMode_ == SamplingMode::NEAREST) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0,
				 GL_RGBA8, mat.cols, mat.rows, 0,
				 GL_BGRA, GL_UNSIGNED_BYTE, mat.data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void ImgviewApp::handleResize(int newWidth, int newHeight)
{
	updateZoom();
	glViewport(0, 0, newWidth, newHeight);
	winAspect_ = float(newWidth) / float(newHeight);
	redraw_ = true;
	winWidth_ = newWidth;
	winHeight_ = newHeight;
	fitImageToWindow();
}

void ImgviewApp::fitImageToWindow()
{
	if(imAspect_ < winAspect_) {
		//Fit vertically
		zoom_ = float(imHeight_) / float(winHeight_);
	} else {
		//Fit horizontally
		zoom_ = float(imWidth_) / float(winWidth_);
	}
	updateZoom();
	centerImageInWindow();
}

void ImgviewApp::fitWindowToImage()
{
	SDL_SetWindowSize(context_.window(), imWidth_, imHeight_);
	handleResize(imWidth_, imHeight_);
}

void ImgviewApp::centerImageInWindow()
{
	offset[0] = -((winWidth_*.5f) - (imWidth_/(multisampleFactor*1.f*zoom_)));
	offset[1] = -winHeight_*.5f - (imHeight_/(multisampleFactor*1.f*zoom_));
	shaderProgram_.setOffset(offset[0], offset[1]);
	redraw_ = true;
}

void ImgviewApp::resetZoom()
{
	zoom_ = 1.f;
	updateZoom();
	redraw_ = true;
}

void ImgviewApp::updateZoom()
{
	shaderProgram_.setFrag2Tex(.5f*zoom_*multisampleFactor/imWidth_,
		-.5f*zoom_*multisampleFactor/imHeight_);
	setTitle();
}

bool ImgviewApp::supportedFileExtension(const std::string &extension)
{
	for(auto &e : supportedExtensions) {
		if (e == extension) {
			return true;
		}
	}
	return false;
}

void ImgviewApp::setTitle()
{
	std::stringstream title;
	title << "imgview: " << imagePaths_[imagePathIdx_].path().filename().string()
		<< " " << imWidth_ << "x" << imHeight_ << "@" << 100.f/zoom_ << "%";
	SDL_SetWindowTitle(context_.window(), title.str().c_str());
}

