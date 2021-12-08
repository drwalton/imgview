#ifndef IMGVIEWAPP_HPP_INCLUDED
#define IMGVIEWAPP_HPP_INCLUDED

#include "SdlGlContext.hpp"
#include "ShaderProgram.hpp"

#include <vector>
#include <boost/filesystem.hpp>
#include <opencv2/opencv.hpp>

class ImgviewApp final
{
public:
	ImgviewApp(const std::string &filename);
	~ImgviewApp() throw();

	void run();

private:
	void loadImagePaths();

	void redrawImage();
	void processEvent(SDL_Event &event);
	bool loadImage(const std::string &filename);
	void handleResize(int newWidth, int newHeight);

	void fitImageToWindow();
	void fitImageToWindowHorz();
	void fitWindowToImage();
	void resetZoom();
	void updateZoom();
	void centerImageInWindow();
	void centerImageInWindowTop();

	void setFullscreen(bool fullscreen);

	void moveToDisplay(int displayIndex);

	bool supportedFileExtension(const std::string &extension);
	
	void setTitle();

	const std::string &listControls() const;

	SdlGlContext context_;
	ShaderProgram shaderProgram_;

	bool redraw_, running_, mouseDown_, rightMouseDown_, alphaBlendEnabled_;
	enum class SamplingMode {LINEAR, NEAREST};
	SamplingMode samplingMode_;
	void updateSamplingMode();
	GLuint texture_;
	float imAspect_, winAspect_, zoom_;
	std::string imFormat_;
	int imWidth_, imHeight_, winWidth_, winHeight_;
	std::vector<boost::filesystem::directory_entry> imagePaths_;
	size_t imagePathIdx_, clearColorIdx_;
	boost::filesystem::path initialImagePath_;
	bool isFullscreen_;
};

#endif
