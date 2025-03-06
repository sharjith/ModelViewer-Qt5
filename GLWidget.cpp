#include "GLWidget.h"

#include "TextRenderer.h"

#include "Cylinder.h"
#include "Cone.h"
#include "Torus.h"
#include "Sphere.h"
#include "Cube.h"
#include "Teapot.h"
#include "KleinBottle.h"
#include "Figure8KleinBottle.h"
#include "BoySurface.h"
#include "TwistedTriaxial.h"
#include "SteinerSurface.h"
#include "SuperToroid.h"
#include "SuperToroidEditor.h"
#include "SuperEllipsoid.h"
#include "SuperEllipsoidEditor.h"
#include "Spring.h"
#include "SpringEditor.h"
#include "AppleSurface.h"
#include "DoubleCone.h"
#include "BentHorns.h"
#include "Folium.h"
#include "LimpetTorus.h"
#include "SaddleTorus.h"
#include "GraysKlein.h"
#include "GraysKleinEditor.h"
#include "BowTie.h"
#include "TriaxialTritorus.h"
#include "TriaxialHexatorus.h"
#include "VerrillMinimal.h"
#include "Horn.h"
#include "Crescent.h"
#include "ConeShell.h"
#include "Periwinkle.h"
#include "TopShell.h"
#include "WrinkledPeriwinkle.h"
#include "SpindleShell.h"
#include "TurretShell.h"
#include "TwistedPseudoSphere.h"
#include "SphericalHarmonicsEditor.h"
#include "ClippingPlanesEditor.h"

#include "Plane.h"
#include "ModelViewer.h"
#include "MainWindow.h"

#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"

#include "AssImpModelLoader.h"

using glm::mat4;
using glm::vec3;

constexpr auto TWO_HUNDRED_MB = 209715200; // bytes

GLWidget::GLWidget(QWidget* parent, const char* /*name*/) : QOpenGLWidget(parent),
_textRenderer(nullptr),
_axisTextRenderer(nullptr),
_sphericalHarmonicsEditor(nullptr),
_superToroidEditor(nullptr),
_superEllipsoidEditor(nullptr),
_springEditor(nullptr),
_graysKleinEditor(nullptr),
_clippingPlanesEditor(nullptr),
_clippingPlaneXY(nullptr),
_clippingPlaneYZ(nullptr),
_clippingPlaneZX(nullptr),
_floorPlane(nullptr),
_skyBox(nullptr),
_axisCone(nullptr),
_lightCube(nullptr),
_assimpModelLoader(nullptr)
{
    setFocusPolicy(Qt::StrongFocus);

    _viewer = static_cast<ModelViewer*>(parent);

    _bgTopColor = QColor::fromRgbF(0.45f, 0.45f, 0.45f, 1.0f);
    _bgBotColor = QColor::fromRgbF(0.9f, 0.9f, 0.9f, 1.0f);

	_quadVAO = 0;

	_viewBoundingSphereDia = 200.0f;
	_viewRange = _viewBoundingSphereDia;
	_FOV = 15.0f;
	_currentViewRange = 1.0f;
	_viewMode = ViewMode::ISOMETRIC;
	_projection = ViewProjection::ORTHOGRAPHIC;

	_autoFitViewOnUpdate = true;

	_primaryCamera = new GLCamera(width(), height(), _viewRange, _FOV);
	_primaryCamera->setView(GLCamera::ViewProjection::SE_ISOMETRIC_VIEW);

	_orthoViewsCamera = new GLCamera(width(), height(), _viewRange, _FOV);
	_orthoViewsCamera->setView(GLCamera::ViewProjection::SE_ISOMETRIC_VIEW);

	_currentRotation = QQuaternion::fromRotationMatrix(_primaryCamera->getViewMatrix().toGenericMatrix<3, 3>());
	_currentTranslation = _primaryCamera->getPosition();
	_currentViewRange = _viewRange;

	_slerpStep = 0.0f;
	_slerpFrac = 0.05f;

	_modelNum = 6;

	_ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
	_diffuseLight = { 1.0f, 1.0f, 1.0f, 1.0f };
	_specularLight = { 0.5f, 0.5f, 0.5f, 1.0f };

	_lightPosition = { 25.0f, 25.0f, 50.0f };
	_lightOffsetX = 0.0f;
	_lightOffsetY = 0.0f;
	_lightOffsetZ = 0.0f;

	_displayMode = DisplayMode::SHADED;
	_renderingMode = RenderingMode::ADS_PHONG;

	_multiViewActive = false;

	_showAxis = true;

	_windowZoomActive = false;
	_rubberBand = nullptr;

	_viewZooming = false;
	_viewPanning = false;
	_viewRotating = false;

	_modelName = "Model";

	_clipYZEnabled = false;
	_clipZXEnabled = false;
	_clipXYEnabled = false;

	_clipXFlipped = false;
	_clipYFlipped = false;
	_clipZFlipped = false;
	_clippingPlaneXY = nullptr;
	_clippingPlaneYZ = nullptr;
	_clippingPlaneZX = nullptr;

	_cappingEnabled = false;

	_showVertexNormals = false;
	_showFaceNormals = false;

	_envMapEnabled = false;
	_shadowsEnabled = false;
	_reflectionsEnabled = false;
	_floorDisplayed = false;
	_floorTextureDisplayed = true;
	_floorTexRepeatS = _floorTexRepeatT = 1;
	_floorOffsetPercent = 0.05f;
	_skyBoxEnabled = false;
	_skyBoxFOV = 45.0f;
	_skyBoxTextureHDRI = false;
	_gammaCorrection = false;
	_screenGamma = 2.2f;
	_hdrToneMapping = false;;

	_lowResEnabled = false;
	_lockLightAndCamera = true;
	_showLights = false;

	_shadowWidth = 1024 * 3;
	_shadowHeight = 1024 * 3;

	_environmentMap = 0;
	_shadowMap = 0;
	_shadowMapFBO = 0;
	_irradianceMap = 0;
	_prefilterMap = 0;
	_brdfLUTTexture = 0;

	_selectionFBO = 0;
	_selectionRBO = 0;
	_selectionDBO = 0;

	_clipXCoeff = 0.0f;
	_clipYCoeff = 0.0f;
	_clipZCoeff = 0.0f;

	_clipDX = 0.0f;
	_clipDY = 0.0f;
	_clipDZ = 0.0f;

	_xTran = 0.0f;
	_yTran = 0.0f;
	_zTran = 0.0f;

	_xRot = 0.0f;
	_yRot = 0.0f;
	_zRot = 0.0f;

	_xScale = 1.0f;
	_yScale = 1.0f;
	_zScale = 1.0f;

	_displayedObjectsMemSize = 0;
	_visibleSwapped = false;

	_keyboardNavTimer = new QTimer(this);
	_keyboardNavTimer->setTimerType(Qt::PreciseTimer);
	connect(_keyboardNavTimer, SIGNAL(timeout()), this, SLOT(performKeyboardNav()));
	_keyboardNavTimer->start(15);

	_animateViewTimer = new QTimer(this);
	_animateViewTimer->setTimerType(Qt::PreciseTimer);
	connect(_animateViewTimer, SIGNAL(timeout()), this, SLOT(animateViewChange()));
	connect(this, SIGNAL(rotationsSet()), this, SLOT(stopAnimations()));

	_animateFitAllTimer = new QTimer(this);
	_animateFitAllTimer->setTimerType(Qt::PreciseTimer);
	connect(_animateFitAllTimer, SIGNAL(timeout()), this, SLOT(animateFitAll()));
	connect(this, SIGNAL(zoomAndPanSet()), this, SLOT(stopAnimations()));

	_animateWindowZoomTimer = new QTimer(this);
	_animateWindowZoomTimer->setTimerType(Qt::PreciseTimer);
	connect(_animateWindowZoomTimer, SIGNAL(timeout()), this, SLOT(animateWindowZoom()));
	connect(this, SIGNAL(zoomAndPanSet()), this, SLOT(stopAnimations()));

	_animateCenterScreenTimer = new QTimer(this);
	_animateCenterScreenTimer->setTimerType(Qt::PreciseTimer);
	connect(_animateCenterScreenTimer, SIGNAL(timeout()), this, SLOT(animateCenterScreen()));
	connect(this, SIGNAL(zoomAndPanSet()), this, SLOT(stopAnimations()));

	_editorLayout = new QVBoxLayout(this);
	_upperLayout = new QFormLayout();
	_upperLayout->setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
	_upperLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
	_upperLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
	_editorLayout->addItem(_upperLayout);

	_editorLayout->addStretch(height());

	_lowerLayout = new QFormLayout();
	_editorLayout->addItem(_lowerLayout);
	_lowerLayout->setFormAlignment(Qt::AlignBottom | Qt::AlignRight);
	_lowerLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
	_lowerLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

	//_displayedObjectsIds.push_back(0);

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));

	_selectRect = new QRubberBand(QRubberBand::Rectangle, this);
}

GLWidget::~GLWidget()
{
	if (_textRenderer)
		delete _textRenderer;
	if (_axisTextRenderer)
		delete _axisTextRenderer;

	for (auto a : _meshStore)
	{
		delete a;
	}
	if (_primaryCamera)	delete _primaryCamera;
	if (_orthoViewsCamera) delete _orthoViewsCamera;

	if (_assimpModelLoader)
		delete _assimpModelLoader;

	cleanUpShaders();

	//std::cout << "GLWidget::~GLWidget : _environmentMap = " << _environmentMap << std::endl;
	glDeleteTextures(1, &_environmentMap);
	//std::cout << "GLWidget::~GLWidget : _shadowMap = " << _shadowMap << std::endl;
	glDeleteTextures(1, &_shadowMap);
	//std::cout << "GLWidget::~GLWidget : _irradianceMap = " << _irradianceMap << std::endl;
	glDeleteTextures(1, &_irradianceMap);
	//std::cout << "GLWidget::~GLWidget : _prefilterMap = " << _prefilterMap << std::endl;
	glDeleteTextures(1, &_prefilterMap);
	//std::cout << "GLWidget::~GLWidget : _brdfLUTTexture = " << _brdfLUTTexture << std::endl;
	glDeleteTextures(1, &_brdfLUTTexture);
	//std::cout << "GLWidget::~GLWidget : _cappingTexture = " << _cappingTexture << std::endl;
	glDeleteTextures(1, &_cappingTexture);

	if (_clippingPlaneXY)
		delete _clippingPlaneXY;
	if (_clippingPlaneYZ)
		delete _clippingPlaneYZ;
	if (_clippingPlaneZX)
		delete _clippingPlaneZX;
	if (_floorPlane)
		delete _floorPlane;
	if (_axisCone)
		delete _axisCone;
	if (_skyBox)
		delete _skyBox;
	if (_lightCube)
		delete _lightCube;

	_axisVBO.destroy();
	_axisVAO.destroy();

	_bgSplitVBO.destroy();
	_bgSplitVAO.destroy();

	_bgVAO.destroy();
}

void GLWidget::cleanUpShaders()
{
	if (_fgShader)	delete _fgShader;
	if (_axisShader) delete _axisShader;
	if (_vertexNormalShader) delete _vertexNormalShader;
	if (_faceNormalShader) delete _faceNormalShader;
	if (_shadowMappingShader) delete _shadowMappingShader;
	if (_skyBoxShader) delete _skyBoxShader;
	if (_irradianceShader) delete _irradianceShader;
	if (_prefilterShader) delete _prefilterShader;
	if (_brdfShader) delete _brdfShader;
	if (_lightCubeShader) delete _lightCubeShader;
	if (_clippingPlaneShader) delete _clippingPlaneShader;
	if (_clippedMeshShader) delete _clippedMeshShader;
	if (_textShader) delete _textShader;
	if (_bgShader) delete _bgShader;
	if (_bgSplitShader) delete _bgSplitShader;
	if (_selectionShader) delete _selectionShader;
	if (_debugShader) delete _debugShader;
}

void GLWidget::updateView()
{
	update();
}

void GLWidget::setTexture(const std::vector<int>& ids, const QImage& texImage)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setTexureImage(QGLWidget::convertToGLFormat(texImage));
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception raised in GLWidget::setTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setSkyBoxTextureFolder(QString folder)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	_skyBoxFaces =
	{
		QString(folder + "/posx"),
		QString(folder + "/negx"),
		QString(folder + "/posz"),
		QString(folder + "/negz"),
		QString(folder + "/posy"),
		QString(folder + "/negy")
	};
	// stb image library supported formats
	QList<QString> supportedFormats = { "jpeg", "jpg", "png", "bmp", "psd", "tga", "gif", "hdr", "pic", "pnm" };

	makeCurrent();

	glBindTexture(GL_TEXTURE_CUBE_MAP, _environmentMap);

	stbi_set_flip_vertically_on_load(true);
	bool loaded = false;
	int width, height, nrComponents;
	void* data = nullptr;
	for (unsigned int i = 0; i < _skyBoxFaces.size(); i++)
	{
		if (!_skyBoxTextureHDRI)
		{
			for (const QString &extn : supportedFormats)
			{
				QString fileName = _skyBoxFaces.at(i) + "." + extn;
				data = static_cast<unsigned char*>(stbi_load(fileName.toStdString().c_str(), &width, &height, &nrComponents, 0));
				if (data)
				{
					loaded = true;
					break;
				}
			}
		}
		else
		{
			QString fileName = _skyBoxFaces.at(i) + ".hdr";
			data = static_cast<float*>(stbi_loadf(fileName.toStdString().c_str(), &width, &height, &nrComponents, 0));
			if (data)
			{
				loaded = true;
			}
		}
		if (loaded)
		{
			if (_skyBoxTextureHDRI)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
			else
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			stbi_image_free(data);
		}
		else
		{
			if (_skyBoxTextureHDRI)
			{
				QMessageBox::critical(this, "Error", "Skybox HDR files are not found in the selected folder\n"
					"Please make sure that if HRDI option is checked,\n"
					"six HDRI images with names in the following manner...\n"
					"posx.hdr, posy.hdr, posz.hdr,\n"
					"negx.hdr, negy.hdr, negz.hdr\n"
					"are present in the folder."
					"\nSkybox has not changed, continuing with the existing one.");
			}
			else
			{
				QString formats;
                for (const QString& extn : supportedFormats)
				{
					formats += extn + " ";
				}
				// Load first image from file
				QMessageBox::critical(this, "Error", "Skybox compatible files are not found in the selected folder.\n"
					"Please make sure that there are six images of supported formats "
					"in the folder with names in the following manner...\n"
					"posx.jpg, posy.jpg, posz.jpg,\n"
					"negx.jpg, negy.jpg, negz.jpg\nSupported formats are:\n" + formats +
					"\nSkybox has not changed, continuing with the existing one.");
			}
			QApplication::restoreOverrideCursor();
			return;
		}
	}
	loadIrradianceMap();
	update();
	QApplication::restoreOverrideCursor();
}

QVector3D GLWidget::getLightPosition() const
{
	return _lightPosition;
}

void GLWidget::setLightOffset(const QVector3D& offset)
{
	_lightOffsetX = offset.x();
	_lightOffsetY = offset.y();
	_lightOffsetZ = offset.z();
}

QVector4D GLWidget::getSpecularLight() const
{
	return _specularLight;
}

void GLWidget::setSpecularLight(const QVector4D& specularLight)
{
	_specularLight = specularLight;
}

QVector4D GLWidget::getDiffuseLight() const
{
	return _diffuseLight;
}

void GLWidget::setDiffuseLight(const QVector4D& diffuseLight)
{
	_diffuseLight = diffuseLight;
}

QVector4D GLWidget::getAmbientLight() const
{
	return _ambientLight;
}

void GLWidget::setAmbientLight(const QVector4D& ambientLight)
{
	_ambientLight = ambientLight;
}

void GLWidget::setViewMode(ViewMode mode)
{
	if (!_animateViewTimer->isActive())
	{
		_keyboardNavTimer->stop();
		_animateViewTimer->start(5);
		_viewMode = mode;
		_slerpStep = 0.0f;
	}
}

void GLWidget::fitAll()
{
	_viewBoundingSphereDia = _boundingSphere.getRadius() * 2;

	if (!_animateFitAllTimer->isActive())
	{
		_keyboardNavTimer->stop();
		_animateFitAllTimer->start(5);
		_slerpStep = 0.0f;
	}
}

void GLWidget::setAutoFitViewOnUpdate(bool update)
{
	_autoFitViewOnUpdate = update;
}

void GLWidget::beginWindowZoom()
{
	_windowZoomActive = true;
	setCursor(QCursor(QPixmap(":/new/prefix1/res/window-zoom-cursor.png"), 12, 12));
}

void GLWidget::performWindowZoom()
{
	_windowZoomActive = false;
	if (_rubberBand)
	{
		QVector3D Z(0, 0, 0); // instead of 0 for x and y we need worldPosition.x() and worldPosition.y() ....
		Z = Z.project(_viewMatrix * _modelMatrix, _projectionMatrix, getViewportFromPoint(_rubberBand->geometry().center()));

		QRect clientRect = getClientRectFromPoint(_rubberBand->geometry().center());
		QPoint clientWinCen = clientRect.center();
		QVector3D o(clientWinCen.x(), height() - clientWinCen.y(), Z.z());
		QVector3D O = o.unproject(_viewMatrix * _modelMatrix, _projectionMatrix, getViewportFromPoint(_rubberBand->geometry().center()));

		QRect zoomRect = _rubberBand->geometry();
		if (zoomRect.width() == 0 || zoomRect.height() == 0)
		{
			emit windowZoomEnded();
			return;
		}
		QPoint zoomWinCen = zoomRect.center();
		QVector3D p(zoomWinCen.x(), height() - zoomWinCen.y(), Z.z());
		QVector3D P = p.unproject(_viewMatrix * _modelMatrix, _projectionMatrix, getViewportFromPoint(_rubberBand->geometry().center()));

		double widthRatio = static_cast<double>(clientRect.width() / zoomRect.width());
		double heightRatio = static_cast<double>(clientRect.height() / zoomRect.height());
		_rubberBandZoomRatio = (heightRatio < widthRatio) ? heightRatio : widthRatio;
		_rubberBandPan = P - O;
	}
	if (!_animateWindowZoomTimer->isActive())
	{
		_keyboardNavTimer->stop();
		_animateWindowZoomTimer->start(5);
		_slerpStep = 0.0f;
	}
	emit windowZoomEnded();
}

void GLWidget::setProjection(ViewProjection proj)
{
	_projection = proj;
	resizeGL(width(), height());
}

void GLWidget::setRotationActive(bool active)
{
	_viewPanning = false;
	_viewZooming = false;
	_viewRotating = active;
	setCursor(QCursor(QPixmap(":/new/prefix1/res/rotatecursor.png")));
	MainWindow::showStatusMessage("Press Esc to deactivate rotation mode");
}

void GLWidget::setPanningActive(bool active)
{
	_viewRotating = false;
	_viewZooming = false;
	_viewPanning = active;
	setCursor(QCursor(QPixmap(":/new/prefix1/res/pancursor.png")));
	MainWindow::showStatusMessage("Press Esc to deactivate panning mode");
}

void GLWidget::setZoomingActive(bool active)
{
	_viewPanning = false;
	_viewRotating = false;
	_viewZooming = active;
	setCursor(QCursor(QPixmap(":/new/prefix1/res/zoomcursor.png")));
	MainWindow::showStatusMessage("Press Esc to deactivate zooming mode");
}

void GLWidget::setDisplayList(const std::vector<int>& ids)
{
	_displayedObjectsIds = ids;

	std::vector<int> allObjectIDs;
	for (size_t i = 0; i < _meshStore.size(); i++)
	{
		allObjectIDs.push_back(static_cast<int>(i));
	}
	_hiddenObjectsIds.clear();
	std::set_difference(
		allObjectIDs.begin(), allObjectIDs.end(),
		_displayedObjectsIds.begin(), _displayedObjectsIds.end(),
		std::back_inserter(_hiddenObjectsIds)
	);

	if (_hiddenObjectsIds.size() == 0 && _visibleSwapped)
		_visibleSwapped = false;

	_currentTranslation = _primaryCamera->getPosition();
	_boundingSphere.setCenter(0, 0, 0);

	unsigned long long memSize = 0;
	for (int i : (_visibleSwapped ? _hiddenObjectsIds : _displayedObjectsIds))
	{
		try
		{
			TriangleMesh* mesh = _meshStore.at(i);
			memSize += mesh->memorySize();
		}
		catch (const std::out_of_range& ex)
		{
			std::cout << ex.what() << std::endl;
		}
	}
	_displayedObjectsMemSize = memSize;
	updateBoundingSphere();

	if (_floorPlane)
	{
		updateFloorPlane();
	}

	if (_autoFitViewOnUpdate)
		fitAll();

	update();

	emit displayListSet();
}

void GLWidget::duplicateObjects(const std::vector<int>& ids)
{
	makeCurrent();
	for (int id : ids)
	{
		TriangleMesh* mesh = _meshStore.at(id);
		if (mesh)
		{
			TriangleMesh* newMesh = mesh->clone();
			if (newMesh)
			{
				addToDisplay(newMesh);
			}
		}
	}
}

void GLWidget::updateBoundingSphere()
{
	_currentTranslation = _primaryCamera->getPosition();
	_boundingSphere.setCenter(0, 0, 0);
	_boundingSphere.setRadius(0.0);

	if ((!_visibleSwapped && _displayedObjectsIds.size() == 0) ||
		(_visibleSwapped && _hiddenObjectsIds.size() == 0))
	{
		_primaryCamera->setPosition(0, 0, 0);
		_currentTranslation = _primaryCamera->getPosition();
		_boundingSphere.setRadius(1.0);
	}
	else
	{
		for (int i : (_visibleSwapped ? _hiddenObjectsIds : _displayedObjectsIds))
		{
			try
			{
				TriangleMesh* mesh = _meshStore.at(i);
				_boundingSphere.addSphere(mesh->getBoundingSphere());
			}
			catch (const std::out_of_range& ex)
			{
				std::cout << ex.what() << std::endl;
			}
		}
	}

	if (_floorPlane)
	{
		updateFloorPlane();
	}

	update();
}

void GLWidget::updateFloorPlane()
{
	_floorSize = _boundingSphere.getRadius();
	_floorCenter = _boundingSphere.getCenter();
	_lightCube->setSize(_boundingSphere.getRadius() * 0.05f);
	_lightPosition.setX(_floorCenter.x() + _boundingSphere.getRadius() * 0.5f + _lightOffsetX);
	_lightPosition.setY(_floorCenter.y() + _boundingSphere.getRadius() * 0.5f + _lightOffsetY);
	_lightPosition.setZ(highestModelZ() + _boundingSphere.getRadius() * 1.5f + (_floorSize * _floorOffsetPercent) + _lightOffsetZ);
	_floorPlane->setPlane(_fgShader, _floorCenter, _floorSize * 4.0f, _floorSize * 4.0f, 1, 1, lowestModelZ() - (_floorSize * _floorOffsetPercent), _floorTexRepeatS, _floorTexRepeatT);
	updateClippingPlane();
}

void GLWidget::updateClippingPlane()
{
	float xside = _clipXFlipped || _clipXCoeff > 0 ? -1.0f : 1.0f;
	float yside = _clipYFlipped || _clipYCoeff > 0 ? 1.0f : -1.0f;
	float zside = _clipZFlipped || _clipZCoeff > 0 ? -1.0f : 1.0f;
	_clippingPlaneXY->setPlane(_clippingPlaneShader, _floorCenter, _floorSize * 100.0f, _floorSize * 100.0f, 1, 1, -_clipZCoeff * zside, _floorSize, _floorSize);
	_clippingPlaneYZ->setPlane(_clippingPlaneShader, _floorCenter, _floorSize * 100.0f, _floorSize * 100.0f, 1, 1, -_clipXCoeff * xside, _floorSize, _floorSize);
	_clippingPlaneZX->setPlane(_clippingPlaneShader, _floorCenter, _floorSize * 100.0f, _floorSize * 100.0f, 1, 1, -_clipYCoeff * yside, _floorSize, _floorSize);
}

void GLWidget::showClippingPlaneEditor(bool show)
{
	if (!_clippingPlanesEditor)
	{
		_clippingPlanesEditor = new ClippingPlanesEditor(this);
	}

	_lowerLayout->addWidget(_clippingPlanesEditor);
	show ? _clippingPlanesEditor->show() : _clippingPlanesEditor->hide();
}

void GLWidget::showAxis(bool show)
{
	_showAxis = show;
	update();
}

void GLWidget::showShadows(bool show)
{
	_shadowsEnabled = show;
	update();
}

void GLWidget::showEnvironment(bool show)
{
	_envMapEnabled = show;
	update();
}

void GLWidget::showSkyBox(bool show)
{
	_skyBoxEnabled = show;
	update();
}

void GLWidget::showReflections(bool show)
{
	_reflectionsEnabled = show;
	update();
}

void GLWidget::showFloor(bool show)
{
	_floorDisplayed = show;
	update();
	emit floorShown(show);
}

void GLWidget::setFloorTexture(QImage img)
{
	_floorTexImage = QGLWidget::convertToGLFormat(img); // flipped 32bit RGBA
	_floorPlane->setTexureImage(_floorTexImage);
}

void GLWidget::showFloorTexture(bool show)
{
	_floorTextureDisplayed = show;
	_floorPlane->enableTexture(_floorTextureDisplayed);
}

void GLWidget::addToDisplay(TriangleMesh* mesh)
{
	_meshStore.push_back(mesh);
	_displayedObjectsIds.push_back(static_cast<int>(_meshStore.size() - 1));
}

void GLWidget::removeFromDisplay(int index)
{
	TriangleMesh* mesh = _meshStore[index];
	_meshStore.erase(_meshStore.begin() + index);
	delete mesh;
	if (_meshStore.size() == 0)
	{
		_displayedObjectsIds.clear();
		_hiddenObjectsIds.clear();
		if (_visibleSwapped)
			_visibleSwapped = false;
	}
}

void GLWidget::centerScreen(std::vector<int> selectedIDs)
{
	_centerScreenObjectIDs.clear();
	_centerScreenObjectIDs = selectedIDs;
	_selectionBoundingSphere.setCenter(0, 0, 0);
	_selectionBoundingSphere.setRadius(0.0);
	if (_displayedObjectsMemSize > TWO_HUNDRED_MB)
		_lowResEnabled = true;
	int count = 0;
	for (int id : _centerScreenObjectIDs)
	{
		TriangleMesh* mesh = _meshStore.at(id);
		if (mesh)
		{
			if (count == 0)
				_selectionBoundingSphere = mesh->getBoundingSphere();
			else
				_selectionBoundingSphere.addSphere(mesh->getBoundingSphere());
		}
		count++;
	}
	if (!_animateCenterScreenTimer->isActive())
	{
		_keyboardNavTimer->stop();
		_animateCenterScreenTimer->start(5);
		_slerpStep = 0.0f;
	}
}

void GLWidget::select(int id)
{
	try {
		_meshStore.at(id)->select();
	}
	catch (const std::exception& ex) {
		std::cout << "Exception raised in GLWidget::select\n" << ex.what() << std::endl;
	}
}

void GLWidget::deselect(int id)
{
	try {
		_meshStore.at(id)->deselect();
	}
	catch (const std::exception& ex) {
		std::cout << "Exception raised in GLWidget::select\n" << ex.what() << std::endl;
	}
}

bool GLWidget::loadAssImpModel(const QString& fileName, QString& error)
{
	bool success = false;

	makeCurrent();
	QString displayFileName = fileName;
	if (fileName.length() > 125)
	{
		displayFileName = fileName.left(61) + "..." + fileName.right(61);
	}
	MainWindow::showStatusMessage("Reading file: " + displayFileName);
	MainWindow::showProgressBar();
	if (_assimpModelLoader)
	{
		/*QMessageBox msgBox(this);
		msgBox.setIcon( QMessageBox::Information);
		msgBox.setText("Click Abort to stop file loading.");
		msgBox.setWindowFlag(Qt::FramelessWindowHint,true);
		msgBox.setModal(false);
		float ph = geometry().height();
		float px = geometry().x();
		float py = geometry().y();
		float dw = msgBox.width();
		float dh = msgBox.height();
		msgBox.setGeometry( px + 20, py+ph-4*dh, dw, dh );
		QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
		connect(abortButton, SIGNAL(pressed()), this, SLOT(cancelAssImpModelLoading()));
		msgBox.show();*/
		_assimpModelLoader->loadModel(const_cast<GLchar*>(fileName.toStdString().c_str()));
		std::vector<AssImpMesh*> meshes = _assimpModelLoader->getMeshes();
		if (meshes.size() == 0)
		{
			success = false;
			error = _assimpModelLoader->getErrorMessage();
		}
		else
		{
			success = true;
			for (AssImpMesh* mesh : meshes)
				addToDisplay(mesh);
		}
	}
	MainWindow::showStatusMessage("");
	MainWindow::setProgressValue(0);
	MainWindow::hideProgressBar();

	return success;
}

void GLWidget::showFileReadingProgress(float percent)
{
	MainWindow::setProgressValue((int)((float)percent * 100.0f));
	makeCurrent();
}

void GLWidget::showMeshLoadingProgress(float /*percent*/)
{
	makeCurrent();
}

void GLWidget::showModelLoadingProgress(int nodeNum, int totalNodes)
{
	MainWindow::showStatusMessage(QString("Processing node: %1/%2").arg(nodeNum).arg(totalNodes));
	MainWindow::setProgressValue((int)((float)nodeNum / (float)totalNodes * 100.0f));
	makeCurrent();
}

void GLWidget::swapVisible(bool checked)
{
	_visibleSwapped = checked;
	updateBoundingSphere();
	if (_autoFitViewOnUpdate)
		fitAll();
	emit visibleSwapped(checked);
}

void GLWidget::cancelAssImpModelLoading()
{
	emit loadingAssImpModelCancelled();
	QMessageBox::critical(this, "Cancelled", "Model loading cancelled!\nModel may be loaded partially");
}

void GLWidget::enableADSDiffuseTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableDiffuseADSMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableADSDiffuseTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setADSDiffuseTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setDiffuseADSMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setADSDiffuseTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearADSDiffuseTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearDiffuseADSMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearADSDiffuseTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enableADSSpecularTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableSpecularADSMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableADSSpecularTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setADSSpecularTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setSpecularADSMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setADSSpecularTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearADSSpecularTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearSpecularADSMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearADSSpecularTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enableADSEmissiveTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableEmissiveADSMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableADSEmissiveTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setADSEmissiveTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setEmissiveADSMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setADSEmissiveTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearADSEmissiveTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearEmissiveADSMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearADSEmissiveTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enableADSNormalTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableNormalADSMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableADSNormalTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setADSNormalTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setNormalADSMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setADSNormalTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearADSNormalTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearNormalADSMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearADSNormalTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enableADSHeightTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableHeightADSMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableADSHeightTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setADSHeightTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setHeightADSMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setADSHeightTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearADSHeightTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearHeightADSMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearADSHeightTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enableADSOpacityTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableOpacityADSMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableADSOpacityTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::invertADSOpacityTexMap(const std::vector<int>& ids, const bool& inverted)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->invertOpacityADSMap(inverted);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::invertADSOpacityTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setADSOpacityTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setOpacityADSMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setADSOpacityTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearADSOpacityTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearOpacityADSMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearADSOpacityTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearADSTexMaps(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearAllADSMaps();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearADSTexMaps\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setMaterialToObjects(const std::vector<int>& ids, const GLMaterial& mat)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setMaterial(mat);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setMaterialToObjects\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setPBRAlbedoColor(const std::vector<int>& ids, const QColor& col)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setPBRAlbedoColor(col.red() / 256.0f, col.green() / 256.0f, col.blue() / 256.0f);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setPBRAlbedoColor\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setPBRMetallic(const std::vector<int>& ids, const float& val)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setPBRMetallic(val);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setPBRMetallic\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setPBRRoughness(const std::vector<int>& ids, const float& val)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setPBRRoughness(val);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setPBRRoughness\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearPBRTexMaps(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearAllPBRMaps();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearPBRTextures\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enablePBRAlbedoTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableAlbedoPBRMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableAlbedoTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setPBRAlbedoTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setAlbedoPBRMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setAlbedoTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearPBRAlbedoTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearAlbedoPBRMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearAlbedoTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enablePBRMetallicTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableMetallicPBRMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableMetallicTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setPBRMetallicTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setMetallicPBRMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setMetallicTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearPBRMetallicTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearMetallicPBRMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearMetallicTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enablePBRRoughnessTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableRoughnessPBRMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableRoughnessTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setPBRRoughnessTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setRoughnessPBRMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setRoughnessTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearPBRRoughnessTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearRoughnessPBRMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearRoughnessTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enablePBRNormalTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableNormalPBRMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableNormalTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setPBRNormalTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setNormalPBRMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setNormalTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearPBRNormalTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearNormalPBRMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearNormalTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enablePBRAOTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableAOPBRMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableAOTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setPBRAOTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setAOPBRMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setAOTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearPBRAOTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearAOPBRMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearAOTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enablePBROpacityTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableOpacityPBRMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enablePBROpacityTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setPBROpacityTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setOpacityPBRMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setPBROpacityTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::invertPBROpacityTexMap(const std::vector<int>& ids, const bool& inverted)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->invertOpacityPBRMap(inverted);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::invertPBROpacityTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearPBROpacityTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearOpacityPBRMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearPBROpacityTexMap\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::enablePBRHeightTexMap(const std::vector<int>& ids, const bool& enable)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->enableHeightPBRMap(enable);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::enableHeightTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setPBRHeightTexMap(const std::vector<int>& ids, const QString& path)
{
	unsigned int texId = loadTextureFromFile(path.toStdString().c_str());
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setHeightPBRMap(texId);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setHeightTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::clearPBRHeightTexMap(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->clearHeightPBRMap();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::clearHeightTexture\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setPBRHeightScale(const std::vector<int>& ids, const float& scale)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setHeightPBRMapScale(scale);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setHeightScale\n" << ex.what() << std::endl;
		}
	}
}

void GLWidget::setTransformation(const std::vector<int>& ids, const QVector3D& trans, const QVector3D& rot, const QVector3D& scale)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->setTranslation(trans);
			mesh->setRotation(rot);
			mesh->setScaling(scale);
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::setTransformation\n" << ex.what() << std::endl;
		}
	}
	updateBoundingSphere();
}

void GLWidget::resetTransformation(const std::vector<int>& ids)
{
	for (int id : ids)
	{
		try
		{
			TriangleMesh* mesh = _meshStore[id];
			mesh->resetTransformations();
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception in GLWidget::resetTransformation\n" << ex.what() << std::endl;
		}
	}
	updateBoundingSphere();
}

void GLWidget::initializeGL()
{
	initializeOpenGLFunctions();

	makeCurrent();

	createShaderPrograms();

	_assimpModelLoader = new AssImpModelLoader(_fgShader);
	connect(_assimpModelLoader, SIGNAL(fileReadProcessed(float)), this, SLOT(showFileReadingProgress(float)));
	connect(_assimpModelLoader, SIGNAL(verticesProcessed(float)), this, SLOT(showMeshLoadingProgress(float)));
    connect(_assimpModelLoader, SIGNAL(nodeProcessed(int,int)), this, SLOT(showModelLoadingProgress(int,int)));
	connect(this, SIGNAL(loadingAssImpModelCancelled()), _assimpModelLoader, SLOT(cancelLoading()));

    QString path = QApplication::applicationDirPath() + "/";
	// Text rendering
	_textShader->bind();
	_textRenderer = new TextRenderer(_textShader, width(), height());
    _textRenderer->Load(path.toStdString() + "fonts/arial.ttf", 20);
	_axisTextRenderer = new TextRenderer(_textShader, width(), height());
    _axisTextRenderer->Load(path.toStdString() + "fonts/arialbd.ttf", 16);
	_textShader->release();

	createCappingPlanes();

	createLights();

	// Environment Mapping
	loadEnvMap();
	// IBL Map
	loadIrradianceMap();
	// Shadow mapping
	loadFloor();

	createGeometry();

	// Set lighting information
	_fgShader->bind();
	_fgShader->setUniformValue("lightSource.ambient", _ambientLight.toVector3D());
	_fgShader->setUniformValue("lightSource.diffuse", _diffuseLight.toVector3D());
	_fgShader->setUniformValue("lightSource.specular", _specularLight.toVector3D());
	_fgShader->setUniformValue("lightSource.position", _lightPosition + QVector3D(_lightOffsetX, _lightOffsetY, _lightOffsetZ));
	_fgShader->setUniformValue("lightModel.ambient", QVector3D(0.2f, 0.2f, 0.2f));
	_fgShader->setUniformValue("texUnit", 0);
	_fgShader->setUniformValue("envMap", 1);
	_fgShader->setUniformValue("shadowMap", 2);
	_fgShader->setUniformValue("irradianceMap", 3);
	_fgShader->setUniformValue("prefilterMap", 4);
	_fgShader->setUniformValue("brdfLUT", 5);

	/*std::vector<int> ids;
	for(size_t i = 0; i < _meshStore.size(); i++)
	{
		ids.push_back(i);
	}
	setAlbedoTexture(ids,       "textures/materials/gold/albedo.png");
	setNormalTexture(ids,       "textures/materials/gold/normal.png");
	setMetallicTexture(ids,     "textures/materials/gold/metallic.png");
	setRoughnessTexture(ids,    "textures/materials/gold/roughness.png");
	setAOTexture(ids,           "textures/materials/gold/ao.png");*/

	_debugShader->bind();
	_debugShader->setUniformValue("depthMap", 0);

	_viewMatrix.setToIdentity();
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 1.f);
}

bool GLWidget::loadCompileAndLinkShaderFromFile(QOpenGLShaderProgram* prog, const QString& vertexProg,
	const QString& fragmentProg, const QString& geometryProg,
	const QString& tessControlProg, const QString& tessEvalProg)
{
	if (prog == nullptr || vertexProg == "" || fragmentProg == "")
		return false;

	bool success = prog->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexProg);
	if (!success)
	{
		qDebug() << "Error in vertex shader:" << prog->objectName() << prog->log();
	}
	if (tessControlProg != "")
	{
		success = prog->addShaderFromSourceFile(QOpenGLShader::TessellationControl, tessControlProg);
		if (!success)
		{
			qDebug() << "Error in tessellation  control shader:" << prog->objectName() << prog->log();
		}
	}
	if (tessEvalProg != "")
	{
		success = prog->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, tessEvalProg);
		if (!success)
		{
			qDebug() << "Error in tessellation  evaluation shader:" << prog->objectName() << prog->log();
		}
	}
	if (geometryProg != "")
	{
		success = prog->addShaderFromSourceFile(QOpenGLShader::Geometry, geometryProg);
		if (!success)
		{
			qDebug() << "Error in geometry shader:" << prog->objectName() << prog->log();
		}
	}
	success = prog->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentProg);
	if (!success)
	{
		qDebug() << "Error in fragment shader:" << prog->objectName() << prog->log();
	}
	if (success)
	{
		success = prog->link();
		if (!success)
		{
			qDebug() << "Error linking shader program:" << prog->objectName() << prog->log();
		}
	}

	return success;
}

void GLWidget::createShaderPrograms()
{
    QString path = QApplication::applicationDirPath() + "/";
	// Foreground objects shader program
	// Per fragment lighting
	_fgShader = new QOpenGLShaderProgram(this); _fgShader->setObjectName("_fgShader");
    loadCompileAndLinkShaderFromFile(_fgShader, path + "shaders/twoside_per_fragment.vert",
        path + "shaders/twoside_per_fragment.frag", path + "shaders/twoside_per_fragment.geom");
	// Axis
	_axisShader = new QOpenGLShaderProgram(this); _axisShader->setObjectName("_axisShader");
    loadCompileAndLinkShaderFromFile(_axisShader, path + "shaders/axis.vert", path + "shaders/axis.frag");
	// Vertex Normal
	_vertexNormalShader = new QOpenGLShaderProgram(this); _vertexNormalShader->setObjectName("_vertexNormalShader");
    loadCompileAndLinkShaderFromFile(_vertexNormalShader, path + "shaders/vertex_normal.vert",
        path + "shaders/vertex_normal.frag", path + "shaders/vertex_normal.geom");
	// Face Normal
	_faceNormalShader = new QOpenGLShaderProgram(this); _faceNormalShader->setObjectName("_faceNormalShader");
    loadCompileAndLinkShaderFromFile(_faceNormalShader, path + "shaders/face_normal.vert",
        path + "shaders/face_normal.frag", path + "shaders/face_normal.geom");
	// Shadow mapping
	_shadowMappingShader = new QOpenGLShaderProgram(this); _shadowMappingShader->setObjectName("_shadowMappingShader");
    loadCompileAndLinkShaderFromFile(_shadowMappingShader, path + "shaders/shadow_mapping_depth.vert",
        path + "shaders/shadow_mapping_depth.frag");
	// Sky Box
	_skyBoxShader = new QOpenGLShaderProgram(this); _skyBoxShader->setObjectName("_skyBoxShader");
    loadCompileAndLinkShaderFromFile(_skyBoxShader, path + "shaders/skybox.vert", path + "shaders/skybox.frag");
	// Irradiance Map
	_irradianceShader = new QOpenGLShaderProgram(this); _irradianceShader->setObjectName("_irradianceShader");
    loadCompileAndLinkShaderFromFile(_irradianceShader, path + "shaders/skybox.vert", path + "shaders/irradiance_convolution.frag");
	// Prefilter Map
	_prefilterShader = new QOpenGLShaderProgram(this); _prefilterShader->setObjectName("_prefilterShader");
    loadCompileAndLinkShaderFromFile(_prefilterShader, path + "shaders/skybox.vert", path + "shaders/prefilter.frag");
	// BRDF LUT Map
	_brdfShader = new QOpenGLShaderProgram(this); _brdfShader->setObjectName("_brdfShader");
    loadCompileAndLinkShaderFromFile(_brdfShader, path + "shaders/brdf.vert", path + "shaders/brdf.frag");
	// Text shader program
	_textShader = new QOpenGLShaderProgram(this); _textShader->setObjectName("_textShader");
    loadCompileAndLinkShaderFromFile(_textShader, path + "shaders/text.vert", path + "shaders/text.frag");
	// Background gradient shader program
	_bgShader = new QOpenGLShaderProgram(this); _bgShader->setObjectName("_bgShader");
    loadCompileAndLinkShaderFromFile(_bgShader, path + "shaders/background.vert", path + "shaders/background.frag");
	// Background split shader program
	_bgSplitShader = new QOpenGLShaderProgram(this); _bgSplitShader->setObjectName("_bgSplitShader");
    loadCompileAndLinkShaderFromFile(_bgSplitShader, path + "shaders/splitScreen.vert", path + "shaders/splitScreen.frag");
	// Light Cube shader program
	_lightCubeShader = new QOpenGLShaderProgram(this); _lightCubeShader->setObjectName("_lightCubeShader");
    loadCompileAndLinkShaderFromFile(_lightCubeShader, path + "shaders/light_cube.vert", path + "shaders/light_cube.frag");
	// Clipping Plane shader program
	_clippingPlaneShader = new QOpenGLShaderProgram(this); _clippingPlaneShader->setObjectName("_clippingPlaneShader");
    loadCompileAndLinkShaderFromFile(_clippingPlaneShader, path + "shaders/clipping_plane.vert", path + "shaders/clipping_plane.frag");
	// Clipped Mesh shader program
	_clippedMeshShader = new QOpenGLShaderProgram(this); _clippedMeshShader->setObjectName("_clippedMeshShader");
    loadCompileAndLinkShaderFromFile(_clippedMeshShader, path + "shaders/clipped_mesh.vert", path + "shaders/clipped_mesh.frag");
	// Selection shader program
	_selectionShader = new QOpenGLShaderProgram(this); _selectionShader->setObjectName("_selectionShader");
    loadCompileAndLinkShaderFromFile(_selectionShader, path + "shaders/selection.vert", path + "shaders/selection.frag");

	// Shadow Depth quad shader program - for debugging
	_debugShader = new QOpenGLShaderProgram(this); _debugShader->setObjectName("_debugShader");
    loadCompileAndLinkShaderFromFile(_debugShader, path + "shaders/debug_quad.vert", path + "shaders/debug_quad_depth.frag");
}

void GLWidget::createCappingPlanes()
{
    QString path = QApplication::applicationDirPath() + "/";
	_clippingPlaneXY = new Plane(_clippingPlaneShader, QVector3D(0, 0, 0), 1000, 1000, 1, 1);
	_clippingPlaneYZ = new Plane(_clippingPlaneShader, QVector3D(0, 0, 0), 1000, 1000, 1, 1);
	_clippingPlaneZX = new Plane(_clippingPlaneShader, QVector3D(0, 0, 0), 1000, 1000, 1, 1);
    _cappingTexture = loadTextureFromFile(QString(path + "textures/patterns/hatch_02.png").toStdString().c_str());
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, _cappingTexture);
}

void GLWidget::createLights()
{
	_lightCube = new Cube(_lightCubeShader, 10);
}

void GLWidget::createGeometry()
{
    /*_meshStore.push_back(new Cube(_fgShader, 100.0f));
	_meshStore.push_back(new Sphere(_fgShader, 50.0f, 100.0f, 100.0f));
	_meshStore.push_back(new Cylinder(_fgShader, 50.0f, 100.0f, 100.0f, 2.0f, 2));
	_meshStore.push_back(new Cone(_fgShader, 50.0f, 100.0f, 100.0f, 2.0f, 2));
	_meshStore.push_back(new Torus(_fgShader, 50.0f, 25.0f, 100.0f, 100.0f, 2, 2));
	_meshStore.push_back(new Teapot(_fgShader, 35.0f, 50, glm::translate(mat4(1.0f), vec3(0.0f, 15.0f, 25.0f))));
	_meshStore.push_back(new KleinBottle(_fgShader, 30.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new Figure8KleinBottle(_fgShader, 30.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new BoySurface(_fgShader, 60.0f, 250.0f, 250.0f, 4, 4));
	_meshStore.push_back(new TwistedTriaxial(_fgShader, 110.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new SteinerSurface(_fgShader, 150.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new AppleSurface(_fgShader, 7.5f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new DoubleCone(_fgShader, 35.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new BentHorns(_fgShader, 15.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new Folium(_fgShader, 75.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new LimpetTorus(_fgShader, 35.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new SaddleTorus(_fgShader, 30.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new BowTie(_fgShader, 40.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new TriaxialTritorus(_fgShader, 45.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new TriaxialHexatorus(_fgShader, 45.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new VerrillMinimal(_fgShader, 25.0f, 150.0f, 150.0f, 1, 1));
	_meshStore.push_back(new Horn(_fgShader, 30.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new Crescent(_fgShader, 30.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new ConeShell(_fgShader, 45.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new Periwinkle(_fgShader, 40.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new TopShell(_fgShader, Point(-50, 0, 0), 35.0f, 250.0f, 150.0f, 4, 4));
	_meshStore.push_back(new WrinkledPeriwinkle(_fgShader, 45.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new SpindleShell(_fgShader, 25.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new TurretShell(_fgShader, 20.0f, 250.0f, 150.0f, 4, 4));
	_meshStore.push_back(new TwistedPseudoSphere(_fgShader, 50.0f, 150.0f, 150.0f, 4, 4));
	_meshStore.push_back(new BreatherSurface(_fgShader, 15.0f, 150.0f, 150.0f, 4, 4));

	Spring* spring = new Spring(_fgShader, 10.0f, 30.0f, 10.0f, 2.0f, 150.0f, 150.0f, 4, 4);
	_meshStore.push_back(spring);
	_springEditor = new SpringEditor(spring, this);
	_upperLayout->addWidget(_springEditor);

	SuperToroid* storoid = new SuperToroid(_fgShader, 50, 25, 1, 1, 150.0f, 150.0f, 4, 4);
	_meshStore.push_back(storoid);
	_superToroidEditor = new SuperToroidEditor(storoid, this);
	_upperLayout->addWidget(_superToroidEditor);

	SuperEllipsoid* sellipsoid = new SuperEllipsoid(_fgShader, 50, 1.0, 1.0, 1.0, 1.0, 1.0, 150.0f, 150.0f, 4, 4);
	_meshStore.push_back(sellipsoid);
	_superEllipsoidEditor = new SuperEllipsoidEditor(sellipsoid, this);
	_upperLayout->addWidget(_superEllipsoidEditor);

	GraysKlein* gklein = new GraysKlein(_fgShader, 30.0f, 150.0f, 150.0f, 4, 4);
	_meshStore.push_back(gklein);
	_graysKleinEditor = new GraysKleinEditor(gklein, this);
	_upperLayout->addWidget(_graysKleinEditor);

	SphericalHarmonic* sph = new SphericalHarmonic(_fgShader, 30.0f, 150.0f, 150.0f, 2, 2);
	_meshStore.push_back(sph);
	_sphericalHarmonicsEditor = new SphericalHarmonicsEditor(sph, this);
    _upperLayout->addWidget(_sphericalHarmonicsEditor);*/

	QString fileName;
#ifdef WIN32
	fileName = "D:/work/progs/qt5/ModelViewer-Github/data/Logo/Logo.stl";
#else
	fileName = "/home/sharjith/work/progs/Qt5/ModelViewer-Github/data/Logo/Logo.stl";
#endif
	//loadAssImpModel(fileName);

	float size = 15;
	_axisCone = new Cone(_axisShader, _viewRange / size / 15, _viewRange / size / 5, 8.0f, 1.0f);
}

void GLWidget::loadFloor()
{
	// configure depth map FBO
	// -----------------------
	// create depth texture
	if (_shadowMap == 0)
	{
		glGenTextures(1, &_shadowMap);
		//std::cout << "GLWidget::loadFloor : _shadowMap = " << _shadowMap << std::endl;
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _shadowMap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, _shadowWidth, _shadowHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// attach depth texture as FBO's depth buffer
		glGenFramebuffers(1, &_shadowMapFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _shadowMapFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _shadowMap, 0);
		unsigned long status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Frame buffer creation failed!" << std::endl;
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
	}

	// Floor texture
    QString path = QApplication::applicationDirPath() + "/";
    if (!_texBuffer.load(QString(path + "textures/envmap/floor/grey-white-checkered-squares1800x1800.jpg")))
	{ // Load first image from file
		qWarning("Could not read image file, using single-color instead.");
		QImage dummy(128, 128, static_cast<QImage::Format>(5));
		dummy.fill(Qt::white);
		_floorTexImage = dummy;
	}
	else
	{
		_floorTexImage = QGLWidget::convertToGLFormat(_texBuffer); // flipped 32bit RGBA
	}

	_floorSize = _boundingSphere.getRadius();
	_floorCenter = _boundingSphere.getCenter();
	_lightPosition.setZ(_floorSize + _lightOffsetZ);
	_floorPlane = new Plane(_fgShader, _floorCenter, _floorSize * 5.0f, _floorSize * 5.0f, 1, 1, -_floorSize - (_floorSize * 0.05f), 1, 1);

	_floorPlane->setAmbientMaterial(QVector3D(0.0f, 0.0f, 0.0f));
	_floorPlane->setDiffuseMaterial(QVector3D(1.0f, 1.0f, 1.0f));
	_floorPlane->setSpecularMaterial(QVector3D(0.5f, 0.5f, 0.5f));
	_floorPlane->setShininess(16.0f);
	_floorPlane->enableTexture(_floorTextureDisplayed);
	_floorPlane->setTexureImage(_floorTexImage);
}

void GLWidget::loadEnvMap()
{
    QString path = QApplication::applicationDirPath() + "/";
	// Env Map
	_skyBoxFaces =
	{
        path + QString("textures/envmap/skyboxes/stormydays/posx.jpg"),
        path + QString("textures/envmap/skyboxes/stormydays/negx.jpg"),
        path + QString("textures/envmap/skyboxes/stormydays/posz.jpg"),
        path + QString("textures/envmap/skyboxes/stormydays/negz.jpg"),
        path + QString("textures/envmap/skyboxes/stormydays/posy.jpg"),
        path + QString("textures/envmap/skyboxes/stormydays/negy.jpg")
	};

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glGenTextures(1, &_environmentMap);
	//std::cout << "GLWidget::loadEnvMap : _environmentMap = " << _environmentMap << std::endl;
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _environmentMap);

	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	void* data = nullptr;
	for (unsigned int i = 0; i < _skyBoxFaces.size(); i++)
	{
		if (_skyBoxTextureHDRI)
			data = static_cast<float*>(stbi_loadf((_skyBoxFaces.at(i)).toStdString().c_str(), &width, &height, &nrComponents, 0));
		else
			data = static_cast<unsigned char*>(stbi_load((_skyBoxFaces.at(i)).toStdString().c_str(), &width, &height, &nrComponents, 0));

		if (data)
		{
			if (_skyBoxTextureHDRI)
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
			else
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			stbi_image_free(data);
		}
		else
		{
			qWarning("Could not read image file, using single-color instead.");
			QImage dummy(128, 128, static_cast<QImage::Format>(5));
			dummy.fill(Qt::white);
			_texImage = dummy;
			_texImage = QGLWidget::convertToGLFormat(_texBuffer); // flipped 32bit RGBA
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, _texImage.width(), _texImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _texImage.bits());
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _environmentMap);

	_skyBox = new Cube(_skyBoxShader, 1);
	_skyBoxShader->bind();
	_skyBoxShader->setUniformValue("skybox", 1);
}

void GLWidget::loadIrradianceMap()
{
	// PBR: setup framebuffer
	// ----------------------
	unsigned int captureFBO;
	unsigned int captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	// PBR: set up projection and view matrices for capturing data onto the 6 cubemap face directions
	// ----------------------------------------------------------------------------------------------
	QMatrix4x4 captureProjection;
	captureProjection.perspective(90.0f, 1.0f, 0.1f, 10.0f);
	QMatrix4x4 view1, view2, view3, view4, view5, view6;
	view1.lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f));
	view2.lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(-1.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f));
	view3.lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f));
	view4.lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, -1.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f));
	view5.lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 1.0f), QVector3D(0.0f, -1.0f, 0.0f));
	view6.lookAt(QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, -1.0f), QVector3D(0.0f, -1.0f, 0.0f));
	QMatrix4x4 captureViews[] = { view1, view2, view3, view4, view5, view6 };

	// PBR: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
	// --------------------------------------------------------------------------------
	if (_irradianceMap)
		glDeleteTextures(1, &_irradianceMap);
	glGenTextures(1, &_irradianceMap);
	//std::cout << "GLWidget::loadIrradianceMap : _irradianceMap = " << _irradianceMap << std::endl;
	glBindTexture(GL_TEXTURE_CUBE_MAP, _irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		if (_skyBoxTextureHDRI)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
		else
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// PBR: solve diffuse integral by convolution to create an irradiance (cube)map.
	// -----------------------------------------------------------------------------
	_skyBox->setProg(_irradianceShader);
	_irradianceShader->bind();
	_irradianceShader->setUniformValue("environmentMap", 1);
	_irradianceShader->setUniformValue("projectionMatrix", captureProjection);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _environmentMap);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		_irradianceShader->bind();
		_irradianceShader->setUniformValue("viewMatrix", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		_skyBox->render();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

	// PBR: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
	// --------------------------------------------------------------------------------
	if (_prefilterMap)
		glDeleteTextures(1, &_prefilterMap);
	glGenTextures(1, &_prefilterMap);
	//std::cout << "GLWidget::loadIrradianceMap : _prefilterMap = " << _prefilterMap << std::endl;
	glBindTexture(GL_TEXTURE_CUBE_MAP, _prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		if (_skyBoxTextureHDRI)
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
		else
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// PBR: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	// ----------------------------------------------------------------------------------------------------
	_skyBox->setProg(_prefilterShader);
	_prefilterShader->bind();
	_prefilterShader->setUniformValue("environmentMap", 1);
	_prefilterShader->setUniformValue("projectionMatrix", captureProjection);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _environmentMap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		_prefilterShader->bind();
		_prefilterShader->setUniformValue("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			_prefilterShader->bind();
			_prefilterShader->setUniformValue("viewMatrix", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, _prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			_skyBox->render();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

	// PBR: generate a 2D LUT from the BRDF equations used.
	// ----------------------------------------------------
	if (_brdfLUTTexture)
		glDeleteTextures(1, &_brdfLUTTexture);
	glGenTextures(1, &_brdfLUTTexture);
	//std::cout << "GLWidget::loadIrradianceMap : _brdfLUTTexture = " << _brdfLUTTexture << std::endl;

	// pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, _brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _brdfLUTTexture, 0);

	glViewport(0, 0, 512, 512);
	_brdfShader->bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

	// bind pre-computed IBL data
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _irradianceMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, _prefilterMap);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, _brdfLUTTexture);
}

void GLWidget::resizeGL(int width, int height)
{
	float w = (float)width;
    float h = (float)height;

	glViewport(0, 0, w, h);
	_viewportMatrix = QMatrix4x4(w / 2, 0.0f, 0.0f, 0.0f,
		0.0f, h / 2, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		w / 2 + 0, h / 2 + 0, 0.0f, 1.0f);

	_projectionMatrix.setToIdentity();
	_primaryCamera->setScreenSize(w, h);
	_primaryCamera->setViewRange(_viewRange);
	if (_projection == ViewProjection::ORTHOGRAPHIC)
	{
		_primaryCamera->setProjectionType(GLCamera::ProjectionType::ORTHOGRAPHIC);
	}
	else
	{
		_primaryCamera->setProjectionType(GLCamera::ProjectionType::PERSPECTIVE);
	}
	_projectionMatrix = _primaryCamera->getProjectionMatrix();
	_viewMatrix = _primaryCamera->getViewMatrix();

	// Resize the text frame
	_textRenderer->setWidth(width);
	_textRenderer->setHeight(height);
	QMatrix4x4 projection;
	projection.ortho(QRect(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h)));
	_textShader->bind();
	_textShader->setUniformValue("projection", projection);
	_textShader->release();

	update();
}

void GLWidget::paintGL()
{
	QColor topColor = !_visibleSwapped ? _bgTopColor : QColor::fromRgbF(1.0f - _bgTopColor.redF(),
		1.0f - _bgTopColor.greenF(), 1.0f - _bgTopColor.blueF(),
		_bgTopColor.alphaF());
	QColor botColor = !_visibleSwapped ? _bgBotColor : QColor::fromRgbF(1.0f - _bgBotColor.redF(),
		1.0f - _bgBotColor.greenF(), 1.0f - _bgBotColor.blueF(),
		_bgBotColor.alphaF());
	try
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		gradientBackground(topColor.redF(), topColor.greenF(), topColor.blueF(), topColor.alphaF(),
			botColor.redF(), botColor.greenF(), botColor.blueF(), botColor.alphaF());

		_modelMatrix.setToIdentity();
		if (_multiViewActive)
		{
            glViewport(0, 0, width(), height());
			if (_shadowsEnabled)
				renderToShadowBuffer();
			gradientBackground(topColor.redF(), topColor.greenF(), topColor.blueF(), topColor.alphaF(),
				botColor.redF(), botColor.greenF(), botColor.blueF(), botColor.alphaF());
			// Render orthographic views with ortho view camera
			// Top View
			_orthoViewsCamera->setScreenSize(width() / 2, height() / 2);
			_orthoViewsCamera->setProjectionMatrix(_projectionMatrix);
			_orthoViewsCamera->setViewMatrix(_viewMatrix);
			_orthoViewsCamera->setPosition(_primaryCamera->getPosition());
			glViewport(0, 0, width() / 2, height() / 2);
			_orthoViewsCamera->setView(GLCamera::ViewProjection::TOP_VIEW);
			render(_orthoViewsCamera);
			_textRenderer->RenderText("Top", -50, 5, 1.6f, glm::vec3(1.0f, 1.0f, 0.0f), TextRenderer::VAlignment::VTOP, TextRenderer::HAlignment::HRIGHT);

			// Front View
			glViewport(0, height() / 2, width() / 2, height() / 2);
			_orthoViewsCamera->setView(GLCamera::ViewProjection::FRONT_VIEW);
			render(_orthoViewsCamera);
			_textRenderer->RenderText("Front", -50, 5, 1.6f, glm::vec3(1.0f, 1.0f, 0.0f), TextRenderer::VAlignment::VTOP, TextRenderer::HAlignment::HRIGHT);

			// Left View
			glViewport(width() / 2, height() / 2, width() / 2, height() / 2);
			_orthoViewsCamera->setView(GLCamera::ViewProjection::LEFT_VIEW);
			render(_orthoViewsCamera);
			_textRenderer->RenderText("Left", -50, 5, 1.6f, glm::vec3(1.0f, 1.0f, 0.0f), TextRenderer::VAlignment::VTOP, TextRenderer::HAlignment::HRIGHT);

			// Render isometric view with primary camera
			// Isometric View
			glViewport(width() / 2, 0, width() / 2, height() / 2);
			render(_primaryCamera);
			std::string viewLabel = _viewMode == ViewMode::DIMETRIC ? "Dimetric" : _viewMode
				== ViewMode::TRIMETRIC ? "Trimetric" : "Isometric";
			_textRenderer->RenderText(viewLabel, -50, 5, 1.6f, glm::vec3(1.0f, 1.0f, 0.0f), TextRenderer::VAlignment::VTOP, TextRenderer::HAlignment::HRIGHT);

			// draw screen partitioning lines
			splitScreen();
		}
		else
		{
			QMatrix4x4 projection;
			projection.ortho(QRect(0.0f, 0.0f, static_cast<float>(width()), static_cast<float>(height())));
			_textShader->bind();
			_textShader->setUniformValue("projection", projection);
			_textShader->release();
			glViewport(0, 0, width(), height());
			if (_shadowsEnabled)
				renderToShadowBuffer();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			gradientBackground(topColor.redF(), topColor.greenF(), topColor.blueF(), topColor.alphaF(),
				botColor.redF(), botColor.greenF(), botColor.blueF(), botColor.alphaF());
			render(_primaryCamera);
			drawCornerAxis();
		}

		// Text rendering
		if (_meshStore.size() != 0 && _displayedObjectsIds.size() != 0)
		{
			int num = _displayedObjectsIds.at(0);
			_textRenderer->RenderText(_meshStore.at(num)->getName().toStdString(), 4, 4, 1, glm::vec3(1.0f, 1.0f, 0.0f));
		}

        /*if (_meshStore.size() && _displayedObjectsIds.size() != 0)
		{
			int num = _displayedObjectsIds[0];
			// Display Harmonics Editor
			if (dynamic_cast<SphericalHarmonic*>(_meshStore.at(num)))
				_sphericalHarmonicsEditor->show();
			else
				_sphericalHarmonicsEditor->hide();

			// Display Gray's Klein Editor
			if (dynamic_cast<GraysKlein*>(_meshStore.at(num)))
				_graysKleinEditor->show();
			else
				_graysKleinEditor->hide();

			// Display Super Toroid Editor
			if (dynamic_cast<SuperToroid*>(_meshStore.at(num)))
				_superToroidEditor->show();
			else
				_superToroidEditor->hide();

			// Display Super Ellipsoid Editor
			if (dynamic_cast<SuperEllipsoid*>(_meshStore.at(num)))
				_superEllipsoidEditor->show();
			else
				_superEllipsoidEditor->hide();

			// Display Spring Editor
			if (dynamic_cast<Spring*>(_meshStore.at(num)))
				_springEditor->show();
			else
				_springEditor->hide();
        }*/
	}
	catch (const std::exception& ex)
	{
		std::cout << "Exception raised in GLWidget::paintGL\n" << ex.what() << std::endl;
	}

	// For testing rendered shadow map
	/*_debugShader.bind();
	_debugShader.setUniformValue("near_plane", 1.0f);
	_debugShader.setUniformValue("far_plane", _viewRange);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, _shadowMap);
	renderQuad();*/

	//_brdfShader->bind();
	//renderQuad();
}

void GLWidget::drawFloor()
{
	if (!_lowResEnabled)
	{
		//https://open.gl/depthstencils
		glEnable(GL_STENCIL_TEST);
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilMask(0x0);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		glStencilMask(0xFF);
		glDepthMask(GL_FALSE);
		glClear(GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);

		// Draw floor
		_fgShader->bind();
		_fgShader->setUniformValue("envMapEnabled", false);
		_fgShader->setUniformValue("floorRendering", true);
		_fgShader->setUniformValue("renderingMode", static_cast<int>(RenderingMode::ADS_PHONG));
		_floorPlane->enableTexture(false);
		_floorPlane->render();
		glDisable(GL_CULL_FACE);

		// Draw model reflection
		glStencilFunc(GL_EQUAL, 1, 0xFF);
		glStencilMask(0x00);
		glDepthMask(GL_TRUE);

		QMatrix4x4 model;
		float floorPos = lowestModelZ() - (_floorSize * _floorOffsetPercent);
		float floorGap = fabs(floorPos - lowestModelZ());
		float offset = ((lowestModelZ()) - floorGap) * 2.0f;
		model.scale(1.0f, 1.0f, -1.0f);
		model.translate(0.0f, 0.0f, -offset);

		_fgShader->bind();
		_fgShader->setUniformValue("modelMatrix", model);
		if (_reflectionsEnabled)
		{
			_fgShader->setUniformValue("renderingMode", static_cast<int>(_renderingMode));
			drawMesh(_fgShader);
		}

		glStencilMask(0x00);
		glDisable(GL_STENCIL_TEST);

		_floorPlane->setOpacity(0.80f);
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	_fgShader->bind();
	_fgShader->setUniformValue("envMapEnabled", _envMapEnabled);
	_fgShader->setUniformValue("renderingMode", static_cast<int>(RenderingMode::ADS_PHONG));
	_fgShader->setUniformValue("shadowSamples", 18.0f);
	_floorPlane->enableTexture(_floorTextureDisplayed);
	_floorPlane->render();
	glDisable(GL_CULL_FACE);
	_fgShader->bind();
	_fgShader->setUniformValue("floorRendering", false);
	_fgShader->setUniformValue("renderingMode", static_cast<int>(_renderingMode));
}

void GLWidget::drawSkyBox()
{
	_skyBox->setProg(_skyBoxShader);
	_skyBoxShader->bind();
	QMatrix4x4 projection;
	projection.perspective(_skyBoxFOV, (float)width() / (float)height(), 0.1f, 100.0f);
	QMatrix4x4 view = _viewMatrix;
	// Remove translation
	view.setColumn(3, QVector4D(0, 0, 0, 1));
	_skyBoxShader->setUniformValue("viewMatrix", view);
	_skyBoxShader->setUniformValue("projectionMatrix", projection);
	_skyBoxShader->setUniformValue("hdrToneMapping", _hdrToneMapping);
	_skyBoxShader->setUniformValue("gammaCorrection", _gammaCorrection);
	_skyBoxShader->setUniformValue("screenGamma", _screenGamma);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	_skyBox->render();
	glDepthFunc(GL_LESS); // set depth function back to default
	glDisable((GL_DEPTH_TEST));
}

void GLWidget::drawMesh(QOpenGLShaderProgram* prog)
{
	QVector3D pos = _primaryCamera->getPosition();

	setupClippingUniforms(prog, pos);

	// Render
	if (_meshStore.size() != 0)
	{
		for (int i : (_visibleSwapped ? _hiddenObjectsIds : _displayedObjectsIds))
		{
			try
			{
				TriangleMesh* mesh = _meshStore.at(i);
				if (mesh)
				{
					mesh->setProg(prog);
					mesh->render();
				}
			}
			catch (const std::exception& ex)
			{
				std::cout << "Exception raised in GLWidget::drawMesh\n" << ex.what() << std::endl;
			}
		}
	}
}

void GLWidget::drawSectionCapping()
{
	// We use a lightweight shader without lighting and stuff for drawing the clipped mesh
	_clippedMeshShader->bind();
	_clippedMeshShader->setUniformValue("modelMatrix", _modelMatrix);
	_clippedMeshShader->setUniformValue("viewMatrix", _viewMatrix);
	_clippedMeshShader->setUniformValue("projectionMatrix", _projectionMatrix);
	for (int i = 0; i < 3; ++i)
	{
		// Clipping Planes
		if (_clipYZEnabled && i == 0)
			glEnable(GL_CLIP_DISTANCE0);
		if (_clipZXEnabled && i == 1)
			glEnable(GL_CLIP_DISTANCE1);
		if (_clipXYEnabled && i == 2)
			glEnable(GL_CLIP_DISTANCE2);

		// https://www.opengl.org/archives/resources/code/samples/advanced/advanced97/notes/node10.html
		// https://glbook.gamedev.net/GLBOOK/glbook.gamedev.net/moglgp/advclip.html
		// https://stackoverflow.com/questions/16901829/how-to-clip-only-intersection-not-union-of-clipping-planes
		// 1) The stencil buffer, color buffer, and depth buffer are cleared,
		glClear(GL_STENCIL_BUFFER_BIT);
		glStencilMask(0x0);
		glDisable(GL_DEPTH_TEST);
		// and color buffer writes are disabled.
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		glEnable(GL_STENCIL_TEST);
		glStencilMask(0xFF);
		glStencilFunc(GL_ALWAYS, 0, 0);

		// 2) The capping polygon is rendered into the depth buffer,
		// drawCappingPlane

		// then depth buffer writes are disabled.
		glDepthMask(GL_FALSE);

		// 3) The stencil operation is set to increment the stencil value where the depth test passes,
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

		// and the model is drawn with glCullFace(GL FRONT).
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		drawMesh(_clippedMeshShader);

		// 4) The stencil operation is then set to decrement the stencil value where the depth test passes,
		glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);

		// and the model is drawn with glCullFace(GL BACK)
		glCullFace(GL_BACK);
		drawMesh(_clippedMeshShader);
		glDisable(GL_CULL_FACE);

		//At this point, the stencil buffer is 1 wherever the clipping plane is enclosed by
		// the frontfacing and backfacing surfaces of the object.
		// 5) The depth buffer is cleared, color buffer writes are enabled,
		glClear(GL_DEPTH_BUFFER_BIT);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		// and the polygon representing the clipping plane is now drawn using whatever material properties are desired,
		// with the stencil function set to GL EQUAL and the reference value set to 1.
		// This draws the color and depth values of the cap into the framebuffer only where the stencil values equal 1.
		glStencilFunc(GL_EQUAL, 1, 0xFF);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		// drawCappingPlane
		{
			QMatrix4x4 model;
			_clippingPlaneShader->bind();
			_clippingPlaneShader->setUniformValue("modelMatrix", model);
			_clippingPlaneShader->setUniformValue("viewMatrix", _viewMatrix);
			_clippingPlaneShader->setUniformValue("projectionMatrix", _projectionMatrix);
			glActiveTexture(GL_TEXTURE13);
			glBindTexture(GL_TEXTURE_2D, _cappingTexture);
			_clippingPlaneShader->setUniformValue("hatchMap", 6);
			float yAng = _clipXFlipped || _clipXCoeff > 0 ? 90.0f : -90.0f;
			float xAng = _clipYFlipped || _clipYCoeff > 0 ? 90.0f : -90.0f;
			float zAng = _clipZFlipped || _clipZCoeff > 0 ? 0.0f : 180.0f;
			// YZ Plane
			model.rotate(yAng, QVector3D(0.0f, 1.0f, 0.0f));
			_clippingPlaneShader->bind();
			_clippingPlaneShader->setUniformValue("modelMatrix", model);
			_clippingPlaneShader->setUniformValue("planeColor", QVector3D(0.20f, 0.5f, 0.5f));
			if (_clipYZEnabled && i == 0)
			{
				_clippingPlaneYZ->render();
			}
			// ZX Plane
			model.setToIdentity();
			model.rotate(xAng, QVector3D(1.0f, 0.0f, 0.0f));
			_clippingPlaneShader->bind();
			_clippingPlaneShader->setUniformValue("modelMatrix", model);
			_clippingPlaneShader->setUniformValue("planeColor", QVector3D(0.5f, 0.20f, 0.5f));
			if (_clipZXEnabled && i == 1)
			{
				_clippingPlaneZX->render();
			}
			// XY Plane
			model.setToIdentity();
			model.rotate(zAng, QVector3D(1.0f, 0.0f, 0.0f));
			_clippingPlaneShader->bind();
			_clippingPlaneShader->setUniformValue("modelMatrix", model);
			_clippingPlaneShader->setUniformValue("planeColor", QVector3D(0.5f, 0.5f, 0.20f));
			if (_clipXYEnabled && i == 2)
			{
				_clippingPlaneXY->render();
			}
		}
		// Clipping Planes
		if (_clipYZEnabled && i == 0)
			glDisable(GL_CLIP_DISTANCE0);
		if (_clipZXEnabled && i == 1)
			glDisable(GL_CLIP_DISTANCE1);
		if (_clipXYEnabled && i == 2)
			glDisable(GL_CLIP_DISTANCE2);
	}
	// 6) Finally, stenciling is disabled, the OpenGL clipping plane is applied, and the
	// clipped object is drawn with color and depth enabled.
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_CULL_FACE);
}

void GLWidget::drawVertexNormals()
{
	QVector3D pos = _primaryCamera->getPosition();
	setupClippingUniforms(_vertexNormalShader, pos);

	if (_meshStore.size() != 0)
	{
		for (int i : (_visibleSwapped ? _hiddenObjectsIds : _displayedObjectsIds))
		{
			if (_showVertexNormals)
			{
				TriangleMesh* mesh = _meshStore.at(i);
				mesh->setProg(_vertexNormalShader);
				mesh->getVAO().bind();
				glDrawElements(GL_TRIANGLES, static_cast<int>(mesh->getPoints().size()), GL_UNSIGNED_INT, 0);
				mesh->getVAO().release();
			}
		}
	}
}

void GLWidget::drawFaceNormals()
{
	QVector3D pos = _primaryCamera->getPosition();
	setupClippingUniforms(_faceNormalShader, pos);

	if (_meshStore.size() != 0)
	{
		for (int i : (_visibleSwapped ? _hiddenObjectsIds : _displayedObjectsIds))
		{
			if (_showFaceNormals)
			{
				TriangleMesh* mesh = _meshStore.at(i);
				mesh->setProg(_faceNormalShader);				
				mesh->getVAO().bind();
				glDrawElements(GL_TRIANGLES, static_cast<int>(mesh->getPoints().size()), GL_UNSIGNED_INT, 0);
				mesh->getVAO().release();
			}
		}
	}
}

void GLWidget::drawAxis()
{
	float size = 15;
	// Labels
	QVector3D xAxis(_viewRange / size, 0, 0);
	xAxis = xAxis.project(_modelViewMatrix, _projectionMatrix, QRect(0, 0, width(), height()));
	_axisTextRenderer->RenderText("X", xAxis.x(), height() - xAxis.y(), 1, glm::vec3(1.0f, 1.0f, 0.0f), TextRenderer::VAlignment::VBOTTOM);

	QVector3D yAxis(0, _viewRange / size, 0);
	yAxis = yAxis.project(_modelViewMatrix, _projectionMatrix, QRect(0, 0, width(), height()));
	_axisTextRenderer->RenderText("Y", yAxis.x(), height() - yAxis.y(), 1, glm::vec3(1.0f, 1.0f, 0.0f), TextRenderer::VAlignment::VBOTTOM);

	QVector3D zAxis(0, 0, _viewRange / size);
	zAxis = zAxis.project(_modelViewMatrix, _projectionMatrix, QRect(0, 0, width(), height()));
	_axisTextRenderer->RenderText("Z", zAxis.x(), height() - zAxis.y(), 1, glm::vec3(1.0f, 1.0f, 0.0f), TextRenderer::VAlignment::VBOTTOM);

	// Axes Lines
	if (!_axisVAO.isCreated())
	{
		_axisVAO.create();
		_axisVAO.bind();
	}

	// Vertex Buffer
	if (!_axisVBO.isCreated())
	{
		_axisVBO = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		_axisVBO.create();
	}
	_axisVBO.bind();
	_axisVBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
	std::vector<float> vertices = {
		0, 0, 0,
		_viewRange / size, 0, 0,
		0, 0, 0,
		0, _viewRange / size, 0,
		0, 0, 0,
		0, 0, _viewRange / size };
	_axisVBO.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

	// Color Buffer
	if (!_axisCBO.isCreated())
	{
		_axisCBO = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		_axisCBO.create();
	}
	_axisCBO.bind();
	_axisCBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
	std::vector<float> colors = {
		1, 0, 0,
		1, 0, 0,
		0, 1, 0,
		0, 1, 0,
		0, 0, 1,
		0, 0, 1 };
	_axisCBO.allocate(colors.data(), static_cast<int>(colors.size() * sizeof(float)));

	_axisShader->bind();

	_axisVBO.bind();
	_axisShader->enableAttributeArray("vertexPosition");
	_axisShader->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

	_axisCBO.bind();
	_axisShader->enableAttributeArray("vertexColor");
	_axisShader->setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);

	_axisShader->setUniformValue("modelViewMatrix", _modelViewMatrix);
	_axisShader->setUniformValue("projectionMatrix", _projectionMatrix);

	_axisShader->setUniformValue("renderCone", false);

	_axisVAO.bind();
	glLineWidth(2.5);
	glDrawArrays(GL_LINES, 0, 6);
	glLineWidth(1);

	// Axes Cones
	// X Axis
	_axisCone->setParameters(_viewRange / size / 15, _viewRange / size / 5, 8.0f, 1.0f);
	_axisShader->setUniformValue("renderCone", true);
	QMatrix4x4 model;
	model.translate(_viewRange / size, 0, 0);
	model.rotate(90, QVector3D(0, 1.0f, 0));
	_axisShader->setUniformValue("coneColor", QVector3D(1.0f, 0.0, 0.0));
	_axisShader->setUniformValue("modelViewMatrix", _viewMatrix * model);
	_axisCone->getVAO().bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_axisCone->getPoints().size()), GL_UNSIGNED_INT, 0);
	_axisCone->getVAO().release();

	// Y Axis
	model.setToIdentity();
	model.translate(0, _viewRange / size, 0);
	model.rotate(90, QVector3D(-1.0f, 0, 0));
	_axisShader->bind();
	_axisShader->setUniformValue("coneColor", QVector3D(0.0, 1.0f, 0.0));
	_axisShader->setUniformValue("modelViewMatrix", _viewMatrix * model);
	_axisCone->getVAO().bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_axisCone->getPoints().size()), GL_UNSIGNED_INT, 0);
	_axisCone->getVAO().release();

	// Z Axis
	model.setToIdentity();
	model.translate(0, 0, _viewRange / size);
	_axisShader->bind();
	_axisShader->setUniformValue("coneColor", QVector3D(0.0, 0.0, 1.0f));
	_axisShader->setUniformValue("modelViewMatrix", _viewMatrix * model);
	_axisCone->getVAO().bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_axisCone->getPoints().size()), GL_UNSIGNED_INT, 0);
	_axisCone->getVAO().release();

	_axisVAO.release();
	_axisShader->release();
}

void GLWidget::drawCornerAxis()
{
	glViewport(width() - width() / 10, height() - height() / 10, width() / 10, height() / 10);
	QMatrix4x4 mat = _modelViewMatrix;
	mat.setColumn(3, QVector4D(0, 0, 0, 1));
	mat.setRow(3, QVector4D(0, 0, 0, 1));

	float size = 3.5;
	// Labels
	QVector3D xAxis(_viewRange / size, 0, 0);
	xAxis = xAxis.project(mat, _projectionMatrix, QRect(0, 0, width(), height()));
	_axisTextRenderer->RenderText("X", xAxis.x(), height() - xAxis.y(), 7, glm::vec3(1.0f, 1.0f, 0.0f), TextRenderer::VAlignment::VBOTTOM);

	QVector3D yAxis(0, _viewRange / size, 0);
	yAxis = yAxis.project(mat, _projectionMatrix, QRect(0, 0, width(), height()));
	_axisTextRenderer->RenderText("Y", yAxis.x(), height() - yAxis.y(), 7, glm::vec3(1.0f, 1.0f, 0.0f), TextRenderer::VAlignment::VBOTTOM);

	QVector3D zAxis(0, 0, _viewRange / size);
	zAxis = zAxis.project(mat, _projectionMatrix, QRect(0, 0, width(), height()));
	_axisTextRenderer->RenderText("Z", zAxis.x(), height() - zAxis.y(), 7, glm::vec3(1.0f, 1.0f, 0.0f), TextRenderer::VAlignment::VBOTTOM);

	// Axes
	if (!_axisVAO.isCreated())
	{
		_axisVAO.create();
		_axisVAO.bind();
	}

	// Vertex Buffer
	if (!_axisVBO.isCreated())
	{
		_axisVBO = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		_axisVBO.create();
	}
	_axisVBO.bind();
	_axisVBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
	std::vector<float> vertices = {
		0, 0, 0,
		_viewRange / size, 0, 0,
		0, 0, 0,
		0, _viewRange / size, 0,
		0, 0, 0,
		0, 0, _viewRange / size };
	_axisVBO.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

	// Color Buffer
	if (!_axisCBO.isCreated())
	{
		_axisCBO = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		_axisCBO.create();
	}
	_axisCBO.bind();
	_axisCBO.setUsagePattern(QOpenGLBuffer::StaticDraw);
	std::vector<float> colors = {
		1, 1, 1,
		1, 1, 1,
		1, 1, 1,
		1, 1, 1,
		1, 1, 1,
		1, 1, 1 };
	_axisCBO.allocate(colors.data(), static_cast<int>(colors.size() * sizeof(float)));

	_axisShader->bind();

	_axisVBO.bind();
	_axisShader->enableAttributeArray("vertexPosition");
	_axisShader->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 3);

	_axisCBO.bind();
	_axisShader->enableAttributeArray("vertexColor");
	_axisShader->setAttributeBuffer("vertexColor", GL_FLOAT, 0, 3);

	_axisShader->setUniformValue("modelViewMatrix", mat);
	_axisShader->setUniformValue("projectionMatrix", _projectionMatrix);

	_axisShader->setUniformValue("renderCone", false);

	_axisVAO.bind();
	glLineWidth(2.0);
	glDrawArrays(GL_LINES, 0, 6);
	glLineWidth(1);

	// Axes Cones
	// X Axis
	_axisCone->setParameters(_viewRange / size / 15, _viewRange / size / 5, 8.0f, 1.0f);
	_axisShader->setUniformValue("renderCone", true);
	mat.translate(_viewRange / size, 0, 0);
	mat.rotate(90, QVector3D(0, 1.0f, 0));
	_axisShader->bind();
	_axisShader->setUniformValue("coneColor", QVector3D(1.0f, 1.0f, 1.0f));
	_axisShader->setUniformValue("modelViewMatrix", mat);
	_axisCone->getVAO().bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_axisCone->getPoints().size()), GL_UNSIGNED_INT, 0);
	_axisCone->getVAO().release();

	// Y Axis
	mat = _modelViewMatrix;
	mat.setColumn(3, QVector4D(0, 0, 0, 1));
	mat.setRow(3, QVector4D(0, 0, 0, 1));
	mat.translate(0, _viewRange / size, 0);
	mat.rotate(90, QVector3D(-1.0f, 0, 0));
	_axisShader->bind();
	_axisShader->setUniformValue("modelViewMatrix", mat);
	_axisCone->getVAO().bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_axisCone->getPoints().size()), GL_UNSIGNED_INT, 0);
	_axisCone->getVAO().release();

	// Z Axis
	mat = _modelViewMatrix;
	mat.setColumn(3, QVector4D(0, 0, 0, 1));
	mat.setRow(3, QVector4D(0, 0, 0, 1));
	mat.translate(0, 0, _viewRange / size);
	_axisShader->bind();
	_axisShader->setUniformValue("modelViewMatrix", mat);
	_axisCone->getVAO().bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(_axisCone->getPoints().size()), GL_UNSIGNED_INT, 0);
	_axisCone->getVAO().release();

	_axisVAO.release();
	_axisShader->release();

	glViewport(0, 0, width(), height());
}

void GLWidget::drawLights()
{
	QMatrix4x4 model;
	model.translate(_lightPosition + QVector3D(_lightOffsetX, _lightOffsetY, _lightOffsetZ));
	_lightCubeShader->bind();
	_lightCubeShader->setUniformValue("modelMatrix", model);
	QMatrix4x4 viewMat = _viewMatrix;
	if (!_lockLightAndCamera)
		viewMat.setColumn(3, QVector4D(0, 0, 0, 1));
	_lightCubeShader->setUniformValue("viewMatrix", viewMat);
	_lightCubeShader->setUniformValue("projectionMatrix", _projectionMatrix);
	_lightCubeShader->setUniformValue("lightColor", _diffuseLight.toVector3D());
	_lightCube->render();
}

void GLWidget::render(GLCamera* camera)
{
	glEnable(GL_DEPTH_TEST);

	_viewMatrix.setToIdentity();
	_viewMatrix = camera->getViewMatrix();
	_projectionMatrix = camera->getProjectionMatrix();

	// model transformations
	/*_modelMatrix.translate(QVector3D(_xTran, _yTran, _zTran));
		_modelMatrix.rotate(_xRot, 1, 0, 0);
		_modelMatrix.rotate(_yRot, 0, 1, 0);
		_modelMatrix.rotate(_zRot, 0, 0, 1);
		_modelMatrix.scale(_xScale, _yScale, _zScale);*/

	_modelViewMatrix = _viewMatrix * _modelMatrix;

	// Check if floor is visible from camera angle to enable/disable shadow
	QVector3D zDir(0.0, 0.0, 1.0);
	QVector3D viewDir = _primaryCamera->getViewDir();
	bool floorVisible = QVector3D::dotProduct(viewDir, zDir) < 0.0f;
	bool showShadows = (_shadowsEnabled && floorVisible && !_lowResEnabled && camera == _primaryCamera);

	_fgShader->bind();
	_fgShader->setUniformValue("lightSource.ambient", _ambientLight.toVector3D());
	_fgShader->setUniformValue("lightSource.diffuse", _diffuseLight.toVector3D());
	_fgShader->setUniformValue("lightSource.specular", _specularLight.toVector3D());
	_fgShader->setUniformValue("lightSource.position", _lightPosition + QVector3D(_lightOffsetX, _lightOffsetY, _lightOffsetZ));
	_fgShader->setUniformValue("lightModel.ambient", QVector3D(0.2f, 0.2f, 0.2f));
	_fgShader->setUniformValue("modelViewMatrix", _modelViewMatrix);
	_fgShader->setUniformValue("normalMatrix", _modelViewMatrix.normalMatrix());
	_fgShader->setUniformValue("projectionMatrix", _projectionMatrix);
	_fgShader->setUniformValue("viewportMatrix", _viewportMatrix);
	_fgShader->setUniformValue("Line.Width", 0.75f);
	_fgShader->setUniformValue("Line.Color", QVector4D(0.05f, 0.0f, 0.05f, 1.0f));
	_fgShader->setUniformValue("displayMode", static_cast<int>(_displayMode));
	_fgShader->setUniformValue("renderingMode", static_cast<int>(_renderingMode));
	_fgShader->setUniformValue("envMapEnabled", _envMapEnabled);
	_fgShader->setUniformValue("shadowsEnabled", showShadows);
	_fgShader->setUniformValue("reflectionMapEnabled", false);
	_fgShader->setUniformValue("cameraPos", _primaryCamera->getPosition());
	_fgShader->setUniformValue("lightPos", _lightPosition + QVector3D(_lightOffsetX, _lightOffsetY, _lightOffsetZ));
	_fgShader->setUniformValue("modelMatrix", _modelMatrix);
	_fgShader->setUniformValue("viewMatrix", _viewMatrix);
	_fgShader->setUniformValue("lightSpaceMatrix", _lightSpaceMatrix);
	_fgShader->setUniformValue("lockLightAndCamera", _lockLightAndCamera);
	_fgShader->setUniformValue("hdrToneMapping", _hdrToneMapping);
	_fgShader->setUniformValue("gammaCorrection", _gammaCorrection);
	_fgShader->setUniformValue("screenGamma", _screenGamma);
	_fgShader->setUniformValue("shadowSamples", 27.0f);

	glPolygonMode(GL_FRONT_AND_BACK, _displayMode == DisplayMode::WIREFRAME ? GL_LINE : GL_FILL);
	glLineWidth(_displayMode == DisplayMode::WIREFRAME ? 1.25 : 1.0);

	// https://stackoverflow.com/questions/16901829/how-to-clip-only-intersection-not-union-of-clipping-planes
	glDisable(GL_STENCIL_TEST);
	if (_clipYZEnabled || _clipZXEnabled || _clipXYEnabled)
	{
		if (_cappingEnabled && !_floorDisplayed)
			drawSectionCapping();
		// Clipping Planes
		if (_clipYZEnabled)
		{
			glEnable(GL_CLIP_DISTANCE0);
			// Mesh
			drawMesh(_fgShader);
			// Vertex Normal
			drawVertexNormals();
			// Face Normal
			drawFaceNormals();
			glDisable(GL_CLIP_DISTANCE0);
		}
		if (_clipZXEnabled)
		{
			glEnable(GL_CLIP_DISTANCE1);
			// Mesh
			drawMesh(_fgShader);
			// Vertex Normal
			drawVertexNormals();
			// Face Normal
			drawFaceNormals();
			glDisable(GL_CLIP_DISTANCE1);
		}
		if (_clipXYEnabled)
		{
			glEnable(GL_CLIP_DISTANCE2);
			// Mesh
			drawMesh(_fgShader);
			// Vertex Normal
			drawVertexNormals();
			// Face Normal
			drawFaceNormals();
			glDisable(GL_CLIP_DISTANCE2);
		}
	}
	else
	{
		// Mesh
		drawMesh(_fgShader);
		// Vertex Normal
		drawVertexNormals();
		// Face Normal
		drawFaceNormals();
	}

	/*
	if (!(_clipDX == 0 && _clipDY == 0 && _clipDZ == 0))
	{
		glEnable(GL_CLIP_DISTANCE3);
	}

	glDisable(GL_CLIP_DISTANCE3);
	*/

	if (_displayMode == DisplayMode::REALSHADED && _floorDisplayed && camera != _orthoViewsCamera)
	{
		drawFloor();
	}

	if (_skyBoxEnabled)
		drawSkyBox();

	if (_showAxis)
		drawAxis();

	if (_showLights)
		drawLights();

	_fgShader->release();
}

void GLWidget::renderToShadowBuffer()
{
	// save current viewport
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	/// Shadow Mapping
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, _shadowWidth, _shadowHeight);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	// 1. render depth of scene to texture (from light's perspective)
	// --------------------------------------------------------------
	QMatrix4x4 lightProjection, lightView;
	float extent = _boundingSphere.getRadius() * 6.0f;
	QVector3D center = _boundingSphere.getCenter();
	float near_plane = 1.0f, far_plane = extent;
	lightProjection.ortho(-extent + center.x(), extent + center.x(),
		-extent + center.y(), extent + center.y(),
		near_plane + center.z(), far_plane + center.z());
	QVector3D lightDir;
	if (_lockLightAndCamera)
		lightDir = QVector3D(center.x(), center.y(), 0);
	else
		lightDir = _lightPosition - QVector3D(_lightOffsetX, _lightOffsetY, _lightOffsetZ) - _primaryCamera->getPosition();
	lightView.lookAt(_lightPosition + QVector3D(_lightOffsetX, _lightOffsetY, _lightOffsetZ), lightDir, QVector3D(0.0, 1.0, 0.0));
	_lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	_shadowMappingShader->bind();
	_shadowMappingShader->setUniformValue("lightSpaceMatrix", _lightSpaceMatrix);
	_shadowMappingShader->setUniformValue("model", _modelMatrix);
	if (_meshStore.size() != 0)
	{
		for (int i : (_visibleSwapped ? _hiddenObjectsIds : _displayedObjectsIds))
		{
			try
			{
				TriangleMesh* mesh = _meshStore.at(i);
				if (mesh)
				{
					mesh->setProg(_shadowMappingShader);
					mesh->getVAO().bind();
					glDrawElements(GL_TRIANGLES, static_cast<int>(mesh->getPoints().size()), GL_UNSIGNED_INT, 0);
					mesh->getVAO().release();
				}
			}
			catch (const std::exception& ex)
			{
				std::cout << "Exception raised in GLWidget::renderToShadowBuffer\n" << ex.what() << std::endl;
			}
		}
	}
	glDisable(GL_CULL_FACE);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
	// End Shadow Mapping
	// restore viewport
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

int GLWidget::processSelection(const QPoint& pixel)
{
	int id = -1;
	if (_selectedIDs.size() != 0)
	{
		if (_selectedIDs.size() == 1)
		{
			id = _selectedIDs.at(0);
		}
		else
		{
			makeCurrent();
			if(_selectionFBO == 0)
				glGenFramebuffers(1, &_selectionFBO);
			glBindFramebuffer(GL_FRAMEBUFFER, _selectionFBO);
			glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 0);
			if(_selectionRBO == 0)
				glGenRenderbuffers(1, &_selectionRBO);
			glBindRenderbuffer(GL_RENDERBUFFER, _selectionRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width(), height());
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _selectionRBO);
			GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, DrawBuffers);
			if(_selectionDBO == 0)
				glGenRenderbuffers(1, &_selectionDBO);
			glBindRenderbuffer(GL_RENDERBUFFER, (GLuint)_selectionDBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width(), height());	
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _selectionDBO);
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "Failed to create selection framebuffer" << std::endl;

			// save current viewport
			int viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glViewport(0, 0, width(), height());
			glBindFramebuffer(GL_FRAMEBUFFER, _selectionFBO);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);			
			_selectionShader->bind();
			_selectionShader->setUniformValue("projectionMatrix", _projectionMatrix);
			_selectionShader->setUniformValue("modelViewMatrix", _modelViewMatrix);

			for (int i : qAsConst(_selectedIDs))
			{
				try
				{
					TriangleMesh* mesh = _meshStore.at(i);
					if (mesh)
					{
						QColor pickColor = indexToColor(i + 1);
						qDebug() << "Id " << i << "Pick Color" << pickColor;
						_selectionShader->bind();
						qreal r, g, b, a;
						pickColor.getRgbF(&r, &g, &b, &a);
						_selectionShader->setUniformValue("pickingColor", QVector4D(r, g, b, a));
						mesh->setProg(_selectionShader);
						mesh->getVAO().bind();
						glDrawElements(GL_TRIANGLES, static_cast<int>(mesh->getPoints().size()), GL_UNSIGNED_INT, 0);
						mesh->getVAO().release();
						glFlush();
						glFinish();
					}
				}
				catch (const std::exception& ex)
				{
					std::cout << "Exception raised in GLWidget::renderToSelectionBuffer\n" << ex.what() << std::endl;
				}
			}			
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			int pixelWinSize = 6;
			std::vector<float> res(static_cast<size_t>(pixelWinSize) * pixelWinSize * 4);
			glReadPixels(pixel.x() - pixelWinSize / 2, viewport[3] - pixel.y() + pixelWinSize / 2, pixelWinSize, pixelWinSize, GL_RGBA, GL_FLOAT, res.data());
			std::map<int, int> voteCount;
			for (size_t i = 0; i < res.size(); i += 4)
			{
				QColor col = QColor::fromRgbF(res[i + 0], res[i + 1], res[i + 2], res[i + 3]);
				//qDebug() << "ReadPixel Color" << col;
				unsigned int colId = colorToIndex(col);
				if (colId != 0)
					voteCount[colId - 1]++;
			}
			if (!voteCount.empty())
				id = std::max_element(voteCount.begin(), voteCount.end(), voteCount.value_comp())->first;

			/*size_t NbBytes = static_cast<size_t>(width()) * height() * 4;
			uchar * pPixelData = new uchar[NbBytes];
			glReadPixels(0, 0, width(), height(), GL_RGBA, GL_UNSIGNED_BYTE, pPixelData);
			QImage image(pPixelData, width(), height(), NbBytes / height(), QImage::Format_RGBA8888);
			image = image.mirrored();
			image.save("d:/ss.bmp");*/

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
			// restore viewport
			glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		}
	}

	return id;
}

void GLWidget::renderQuad()
{
	if (_quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &_quadVAO);
		glGenBuffers(1, &_quadVBO);
		glBindVertexArray(_quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(_quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void GLWidget::gradientBackground(float top_r, float top_g, float top_b, float top_a,
	float bot_r, float bot_g, float bot_b, float bot_a)
{
	glViewport(0, 0, width(), height());
	if (!_bgVAO.isCreated())
	{
		_bgVAO.create();
	}

	glDisable(GL_DEPTH_TEST);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	_bgShader->bind();

	_bgShader->setUniformValue("top_color", QVector4D(top_r, top_g, top_b, top_a));
	_bgShader->setUniformValue("bot_color", QVector4D(bot_r, bot_g, bot_b, bot_a));

	_bgVAO.bind();
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glEnable(GL_DEPTH_TEST);

	_bgVAO.release();
	_bgShader->release();
}

void GLWidget::splitScreen()
{
	if (!_bgSplitVAO.isCreated())
	{
		_bgSplitVAO.create();
		_bgSplitVAO.bind();
	}

	if (!_bgSplitVBO.isCreated())
	{
		_bgSplitVBO = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		_bgSplitVBO.create();
		_bgSplitVBO.bind();
		_bgSplitVBO.setUsagePattern(QOpenGLBuffer::StaticDraw);

		static const std::vector<float> vertices = {
			-static_cast<float>(width()) / 2,
			0,
			static_cast<float>(width()) / 2,
			0,
			0,
			-static_cast<float>(height()) / 2,
			0,
			static_cast<float>(height()) / 2,
		};

		_bgSplitVBO.allocate(vertices.data(), static_cast<int>(vertices.size() * sizeof(float)));

		_bgSplitShader->bind();
		_bgSplitShader->enableAttributeArray("vertexPosition");
		_bgSplitShader->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2);

		_bgSplitVBO.release();
	}

	glViewport(0, 0, width(), height());

	glDisable(GL_DEPTH_TEST);

	_bgSplitVAO.bind();
	glLineWidth(0.5);
	glDrawArrays(GL_LINES, 0, 4);
	glLineWidth(1);

	glEnable(GL_DEPTH_TEST);

	_bgSplitVAO.release();
	_bgSplitShader->release();
}

void GLWidget::setupClippingUniforms(QOpenGLShaderProgram* prog, QVector3D pos)
{
	prog->bind();
	if (_clipYZEnabled || _clipZXEnabled || _clipXYEnabled || !(_clipDX == 0 && _clipDY == 0 && _clipDZ == 0))
	{
		_fgShader->setUniformValue("sectionActive", true);
	}
	else
	{
		_fgShader->setUniformValue("sectionActive", false);
	}
	prog->setUniformValue("modelViewMatrix", _modelViewMatrix);
	prog->setUniformValue("projectionMatrix", _projectionMatrix);
	prog->setUniformValue("clipPlaneX", QVector4D(_modelViewMatrix * (QVector3D(_clipXFlipped ? 1 : -1, 0, 0) + pos),
		(_clipXFlipped ? 1 : -1) * (pos.x() - _clipXCoeff)));
	prog->setUniformValue("clipPlaneY", QVector4D(_modelViewMatrix * (QVector3D(0, _clipYFlipped ? 1 : -1, 0) + pos),
		(_clipYFlipped ? 1 : -1) * (pos.y() - _clipYCoeff)));
	prog->setUniformValue("clipPlaneZ", QVector4D(_modelViewMatrix * (QVector3D(0, 0, _clipZFlipped ? 1 : -1) + pos),
		(_clipZFlipped ? 1 : -1) * (pos.z() - _clipZCoeff)));
	prog->setUniformValue("clipPlane", QVector4D(_modelViewMatrix * (QVector3D(_clipDX, _clipDY, _clipDZ) + pos),
		pos.x() * _clipDX + pos.y() * _clipDY + pos.z() * _clipDZ));
}

void GLWidget::checkAndStopTimers()
{
	if (_animateViewTimer->isActive())
	{
		_animateViewTimer->stop();
		// Set all defaults
		_currentRotation = QQuaternion::fromRotationMatrix(_primaryCamera->getViewMatrix().toGenericMatrix<3, 3>());
		_currentTranslation = _primaryCamera->getPosition();
		_currentViewRange = _viewRange;
		_slerpStep = 0.0f;
		emit rotationsSet();
	}
	if (_animateFitAllTimer->isActive())
	{
		_animateFitAllTimer->stop();
		// Set all defaults
		_currentTranslation = _primaryCamera->getPosition();
		_currentViewRange = _viewRange;
		_slerpStep = 0.0f;
		emit zoomAndPanSet();
	}
	if (_animateWindowZoomTimer->isActive())
	{
		_animateWindowZoomTimer->stop();
		_animateFitAllTimer->stop();
		// Set all defaults
		_currentTranslation = _primaryCamera->getPosition();
		_currentViewRange = _viewRange;
		_slerpStep = 0.0f;
		emit zoomAndPanSet();
	}
	if (_animateCenterScreenTimer->isActive())
	{
		_animateCenterScreenTimer->stop();
		_animateFitAllTimer->stop();
		// Set all defaults
		_currentTranslation = _primaryCamera->getPosition();
		_currentViewRange = _viewRange;
		_slerpStep = 0.0f;
		emit zoomAndPanSet();
	}
}

void GLWidget::disableLowRes()
{
	_lowResEnabled = false;
	update();
}

void GLWidget::lockLightAndCamera(bool lock)
{
	_lockLightAndCamera = lock;
	update();
}

void GLWidget::mousePressEvent(QMouseEvent* e)
{
	setFocus();
	checkAndStopTimers();
	if (e->button() & Qt::LeftButton)
	{
		_leftButtonPoint.setX(e->x());
		_leftButtonPoint.setY(e->y());

		if (!(e->modifiers() & Qt::ControlModifier) && !(e->modifiers() & Qt::ShiftModifier)
			&& !_windowZoomActive && !_viewRotating && !_viewPanning && !_viewZooming)
		{
			// Selection
			clickSelect(QPoint(e->x(), e->y()));
		}

		if (!_rubberBand)
		{
			_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
			_rubberBand->setStyle(QStyleFactory::create("Fusion"));
		}
		_rubberBand->setGeometry(QRect(_leftButtonPoint, QSize()));
		_rubberBand->show();
	}

	if ((e->button() & Qt::RightButton) || ((e->button() & Qt::LeftButton) && _viewPanning))
	{
		_rightButtonPoint.setX(e->x());
		_rightButtonPoint.setY(e->y());
	}

	if (e->button() & Qt::MiddleButton || ((e->button() & Qt::LeftButton) && _viewRotating))
	{
		_middleButtonPoint.setX(e->x());
		_middleButtonPoint.setY(e->y());
	}
}

void GLWidget::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() & Qt::LeftButton)
	{
		_rubberBand->hide();
		if (_windowZoomActive)
		{
			performWindowZoom();
		}
		else if (!(e->modifiers() & Qt::ControlModifier) && !_viewRotating && !_viewPanning && !_viewZooming)
		{
			sweepSelect(e->pos());
		}
	}

	if (e->button() & Qt::RightButton)
	{
	}

	if (e->button() & Qt::MiddleButton)
	{
		if (e->modifiers() == Qt::NoModifier)
		{
			if (!_multiViewActive)
			{
				QPoint o(width() / 2, height() / 2);
				QPoint p = e->pos();

				QVector3D OP = get3dTranslationVectorFromMousePoints(o, p);
				_primaryCamera->move(OP.x(), OP.y(), OP.z());
				_currentTranslation = _primaryCamera->getPosition();
				update();
			}
		}
	}

	_lowResEnabled = false;
	if (!_viewRotating && !_viewPanning && !_viewZooming)
	{
		setCursor(QCursor(Qt::ArrowCursor));
	}
	update();
}

void GLWidget::mouseMoveEvent(QMouseEvent* e)
{
	QPoint downPoint(e->x(), e->y());
	if (e->buttons() == Qt::LeftButton && !_viewPanning && !_viewZooming)
	{
		if (!(e->modifiers() & Qt::ControlModifier) && !_viewRotating && !_viewPanning && !_viewZooming)
		{
			_rubberBand->setGeometry(QRect(_leftButtonPoint, e->pos()).normalized());
		}
		if (_windowZoomActive)
		{
			setCursor(QCursor(QPixmap(":/new/prefix1/res/window-zoom-cursor.png"), 12, 12));
		}
		else if ((e->modifiers() & Qt::ControlModifier) || _viewRotating)
		{
			if (_displayedObjectsMemSize > TWO_HUNDRED_MB)
				_lowResEnabled = true;
			QPoint rotate = _leftButtonPoint - downPoint;

			_primaryCamera->rotateX(rotate.y() / 2.0);
			_primaryCamera->rotateY(rotate.x() / 2.0);
			_currentRotation = QQuaternion::fromRotationMatrix(_primaryCamera->getViewMatrix().toGenericMatrix<3, 3>());
			_leftButtonPoint = downPoint;
			setCursor(QCursor(QPixmap(":/new/prefix1/res/rotatecursor.png")));
			_viewMode = ViewMode::NONE;
		}
	}
	else if ((e->buttons() == Qt::RightButton && e->modifiers() & Qt::ControlModifier) || (e->buttons() == Qt::LeftButton && _viewPanning))
	{
		if (_displayedObjectsMemSize > TWO_HUNDRED_MB)
			_lowResEnabled = true;
		QVector3D OP = get3dTranslationVectorFromMousePoints(downPoint, _rightButtonPoint);
		_primaryCamera->move(OP.x(), OP.y(), OP.z());
		_currentTranslation = _primaryCamera->getPosition();

		_rightButtonPoint = downPoint;
		setCursor(QCursor(QPixmap(":/new/prefix1/res/pancursor.png")));
	}
	else if ((e->buttons() == Qt::MiddleButton && e->modifiers() & Qt::ControlModifier) || (e->buttons() == Qt::LeftButton && _viewZooming))
	{
		if (_displayedObjectsMemSize > TWO_HUNDRED_MB)
			_lowResEnabled = true;
		// Zoom
		if (downPoint.x() > _middleButtonPoint.x() || downPoint.y() < _middleButtonPoint.y())
			_viewRange /= 1.05f;
		else
			_viewRange *= 1.05f;
		if (_viewRange < _boundingSphere.getRadius() / 100.0f)
			_viewRange = _boundingSphere.getRadius() / 100.0f;
		if (_viewRange > _boundingSphere.getRadius() * 100.0f)
			_viewRange = _boundingSphere.getRadius() * 100.0f;
		_currentViewRange = _viewRange;

		// Translate to focus on mouse center
		QPoint cen = getClientRectFromPoint(downPoint).center();
		float sign = (downPoint.x() > _middleButtonPoint.x() || downPoint.y() < _middleButtonPoint.y()) ? 1.0f : -1.0f;
		QVector3D OP = get3dTranslationVectorFromMousePoints(cen, _middleButtonPoint);
		OP *= sign * 0.05f;
		_primaryCamera->move(OP.x(), OP.y(), OP.z());
		_currentTranslation = _primaryCamera->getPosition();

		resizeGL(width(), height());

		_middleButtonPoint = downPoint;
		setCursor(QCursor(QPixmap(":/new/prefix1/res/zoomcursor.png")));
	}
	else
	{
		_lowResEnabled = false;
	}
	update();
}

void GLWidget::wheelEvent(QWheelEvent* e)
{
	if (_displayedObjectsMemSize > TWO_HUNDRED_MB)
		_lowResEnabled = true;
	// Zoom
	QPoint numDegrees = e->angleDelta() / 8;
	QPoint numSteps = numDegrees / 15;
	float zoomStep = numSteps.y();
	float zoomFactor = abs(zoomStep) + 0.05;

	if (zoomStep < 0)
		_viewRange *= zoomFactor;
	else
		_viewRange /= zoomFactor;
	if (_viewRange < _boundingSphere.getRadius() / 100.0f)
		_viewRange = _boundingSphere.getRadius() / 100.0f;
	if (_viewRange > _boundingSphere.getRadius() * 100.0f)
		_viewRange = _boundingSphere.getRadius() * 100.0f;
	_currentViewRange = _viewRange;

	// Translate to focus on mouse center
	QPoint cen = getClientRectFromPoint(e->position().toPoint()).center();
	float sign = (e->position().x() > cen.x() || e->position().y() < cen.y() ||
		(e->position().x() < cen.x() && e->position().y() > cen.y())) && (zoomStep > 0) ? 1.0f : -1.0f;
	QVector3D OP = get3dTranslationVectorFromMousePoints(cen, e->position().toPoint());
	OP *= sign * 0.05f;
	_primaryCamera->move(OP.x(), OP.y(), OP.z());
	_currentTranslation = _primaryCamera->getPosition();

	resizeGL(width(), height());
	update();
}

void GLWidget::keyPressEvent(QKeyEvent* event)
{
	_keys[event->key()] = true;
	QWidget::keyPressEvent(event);

	if (_keys[Qt::Key_Escape])
	{
		_viewRotating = false;
		_viewPanning = false;
		_viewZooming = false;
		_windowZoomActive = false;
		setCursor(QCursor(Qt::ArrowCursor));
		MainWindow::showStatusMessage("");

		_viewer->deselectAll();
	}

	if (_keys[Qt::Key_F])
		fitAll();
	if (_keys[Qt::Key_Delete])
		_viewer->deleteSelectedItems();
	if (_keys[Qt::Key_Space])
	{
		if (event->modifiers() & Qt::ShiftModifier)
			_viewer->showOnlySelectedItems();
		else
			_visibleSwapped ? _viewer->showSelectedItems() : _viewer->hideSelectedItems();
	}
	if (_keys[Qt::Key_S])
	{
		if (event->modifiers() & Qt::AltModifier)
			swapVisible(!_visibleSwapped);
	}

	update();
}

void GLWidget::keyReleaseEvent(QKeyEvent* event)
{
	_keys[event->key()] = false;
	QWidget::keyReleaseEvent(event);
}

void GLWidget::performKeyboardNav()
{
	if (QApplication::keyboardModifiers() == Qt::NoModifier)
	{
		float factor = _viewRange * 0.01f;
		// https://forum.qt.io/topic/28327/big-issue-with-qt-key-inputs-for-gaming/4
		if (_primaryCamera->getProjectionType() == GLCamera::ProjectionType::PERSPECTIVE)
		{
			if (_keys[Qt::Key_A])
				_primaryCamera->moveAcross(factor);
			if (_keys[Qt::Key_D])
				_primaryCamera->moveAcross(-factor);
			if (_keys[Qt::Key_W])
				_primaryCamera->moveForward(-factor);
			if (_keys[Qt::Key_S])
				_primaryCamera->moveForward(factor);
		}
		else
		{
			if (_keys[Qt::Key_A])
				_primaryCamera->moveAcross(factor);
			if (_keys[Qt::Key_D])
				_primaryCamera->moveAcross(-factor);
			if (_keys[Qt::Key_W])
				_primaryCamera->moveUpward(-factor);
			if (_keys[Qt::Key_S])
				_primaryCamera->moveUpward(factor);
		}

		if (_keys[Qt::Key_J])
			_primaryCamera->rotateY(2.0f);
		if (_keys[Qt::Key_L])
			_primaryCamera->rotateY(-2.0f);
		if (_keys[Qt::Key_I])
			_primaryCamera->rotateX(2.0f);
		if (_keys[Qt::Key_K])
			_primaryCamera->rotateX(-2.0f);
		if (_keys[Qt::Key_M])
			_primaryCamera->rotateZ(2.0f);
		if (_keys[Qt::Key_N])
			_primaryCamera->rotateZ(-2.0f);
		if (_keys[Qt::Key_Q] || _keys[Qt::Key_Z])
		{
			// Zoom
			if (_keys[Qt::Key_Q])
				_viewRange /= 1.05f;
			else
				_viewRange *= 1.05f;
			if (_viewRange < _boundingSphere.getRadius() / 100.0f)
				_viewRange = _boundingSphere.getRadius() / 100.0f;
			if (_viewRange > _boundingSphere.getRadius() * 100.0f)
				_viewRange = _boundingSphere.getRadius() * 100.0f;
			// Translate to focus on mouse center
			QPoint pos = mapFromGlobal(QCursor::pos());
			QPoint cen = getClientRectFromPoint(pos).center();
			float sign = (pos.x() > cen.x() || pos.y() < cen.y() ||
				(pos.x() < cen.x() && pos.y() > cen.y())) && _keys[Qt::Key_Q] ? 1.0f : -1.0f;
			QVector3D OP = get3dTranslationVectorFromMousePoints(cen, pos);
            OP *= sign * 0.05f;
			_primaryCamera->move(OP.x(), OP.y(), OP.z());
		}

		_currentViewRange = _viewRange;
		_currentTranslation = _primaryCamera->getPosition();
		_currentRotation = QQuaternion::fromRotationMatrix(_primaryCamera->getViewMatrix().toGenericMatrix<3, 3>());
		resizeGL(width(), height());
		update();
	}
}

void GLWidget::animateViewChange()
{
	if (_displayedObjectsMemSize > TWO_HUNDRED_MB)
		_lowResEnabled = true;
	if (_viewMode == ViewMode::TOP)
	{
		setRotations(0.0f, 0.0f, 0.0f);
	}
	if (_viewMode == ViewMode::BOTTOM)
	{
		setRotations(0.0f, 180.0f, 0.0f);
	}
	if (_viewMode == ViewMode::LEFT)
	{
		setRotations(0.0f, -90.0f, 90.0f);
	}
	if (_viewMode == ViewMode::RIGHT)
	{
		setRotations(0.0f, -90.0f, -90.0f);
	}
	if (_viewMode == ViewMode::FRONT)
	{
		setRotations(0.0f, -90.0f, 0.0f);
	}
	if (_viewMode == ViewMode::BACK)
	{
		setRotations(0.0f, -90.0f, 180.0f);
	}
	if (_viewMode == ViewMode::ISOMETRIC)
	{
        setRotations(-45.0f, -54.7356f, 0.0f);
	}
	if (_viewMode == ViewMode::DIMETRIC)
	{
        setRotations(-20.7048f, -70.5288f, 0.0f);
	}
	if (_viewMode == ViewMode::TRIMETRIC)
	{
        setRotations(-30.0f, -55.0f, 0.0f);
	}

	resizeGL(width(), height());
}

void GLWidget::animateFitAll()
{
	if (_displayedObjectsMemSize > TWO_HUNDRED_MB)
		_lowResEnabled = true;
	setZoomAndPan(_viewBoundingSphereDia, -_currentTranslation + _boundingSphere.getCenter());
	resizeGL(width(), height());
}

void GLWidget::animateWindowZoom()
{
	if (_displayedObjectsMemSize > TWO_HUNDRED_MB)
		_lowResEnabled = true;
	setZoomAndPan(_currentViewRange / _rubberBandZoomRatio, _rubberBandPan);
	resizeGL(width(), height());
}

void GLWidget::animateCenterScreen()
{
	setZoomAndPan(_selectionBoundingSphere.getRadius() * 2, -_currentTranslation + _selectionBoundingSphere.getCenter());
	resizeGL(width(), height());
}

void GLWidget::stopAnimations()
{
	_animateViewTimer->stop();
	_animateFitAllTimer->stop();
	_animateWindowZoomTimer->stop();
	_animateCenterScreenTimer->stop();
	_keyboardNavTimer->start();
	QTimer::singleShot(100, this, SLOT(disableLowRes()));
}

void GLWidget::convertClickToRay(const QPoint& pixel, const QRect& viewport, GLCamera* camera, QVector3D& orig, QVector3D& dir)
{
	if (_projection == ViewProjection::PERSPECTIVE)
	{
		QVector3D Z(0, 0, -_viewRange); // instead of 0 for x and y we need worldPosition.x() and worldPosition.y() ....
		Z = Z.project(_viewMatrix * _modelMatrix, camera->getProjectionMatrix(), viewport);
		QVector3D p(pixel.x(), height() - pixel.y() - 1, Z.z());
		QVector3D P = p.unproject(_viewMatrix * _modelMatrix, camera->getProjectionMatrix(), viewport);

		orig = QVector3D(P.x(), P.y(), P.z());

		QVector3D Z1(0, 0, _viewRange); // instead of 0 for x and y we need worldPosition.x() and worldPosition.y() ....
		Z1 = Z1.project(_viewMatrix * _modelMatrix, camera->getProjectionMatrix(), viewport);
		QVector3D q(pixel.x(), height() - pixel.y() - 1, Z1.z());
		QVector3D Q = q.unproject(_viewMatrix * _modelMatrix, camera->getProjectionMatrix(), viewport);

		dir = (Q - P).normalized();
	}
	else
	{
		QVector3D nearPoint(pixel.x(), height() - pixel.y() - 1, 0.0f);
		QVector3D farPoint(pixel.x(), height() - pixel.y() - 1, 1.0f);
		orig = nearPoint.unproject(_viewMatrix * _modelMatrix, camera->getProjectionMatrix(), viewport);
		dir = farPoint.unproject(_viewMatrix * _modelMatrix, camera->getProjectionMatrix(), viewport) - orig;
	}
}

QRect GLWidget::getViewportFromPoint(const QPoint& pixel)
{
	QRect viewport;
	if (_multiViewActive)
	{
		// top view
		if (pixel.x() < width() / 2 && pixel.y() > height() / 2)
			viewport = QRect(0, 0, width() / 2, height() / 2);
		// front view
		if (pixel.x() < width() / 2 && pixel.y() < height() / 2)
			viewport = QRect(0, height() / 2, width() / 2, height() / 2);
		// left view
		if (pixel.x() > width() / 2 && pixel.y() < height() / 2)
			viewport = QRect(width() / 2, height() / 2, width() / 2, height() / 2);
		// isometric
		if (pixel.x() > width() / 2 && pixel.y() > height() / 2)
			viewport = QRect(width() / 2, 0, width() / 2, height() / 2);
	}
	else
	{
		// single viewport
		viewport = QRect(0, 0, width(), height());
	}

	return viewport;
}

QRect GLWidget::getClientRectFromPoint(const QPoint& pixel)
{
	QRect clientRect;
	if (_multiViewActive)
	{
		// top view
		if (pixel.x() < width() / 2 && pixel.y() > height() / 2)
			clientRect = QRect(0, height() / 2, width() / 2, height() / 2);
		// front view
		if (pixel.x() < width() / 2 && pixel.y() < height() / 2)
			clientRect = QRect(0, 0, width() / 2, height() / 2);
		// left view
		if (pixel.x() > width() / 2 && pixel.y() < height() / 2)
			clientRect = QRect(width() / 2, 0, width() / 2, height() / 2);
		// isometric
		if (pixel.x() > width() / 2 && pixel.y() > height() / 2)
			clientRect = QRect(width() / 2, height() / 2, width() / 2, height() / 2);
	}
	else
	{
		// single viewport
		clientRect = QRect(0, 0, width(), height());
	}

	return clientRect;
}

QVector3D GLWidget::get3dTranslationVectorFromMousePoints(const QPoint& start, const QPoint& end)
{
	QVector3D Z(0, 0, 0); // instead of 0 for x and y we need worldPosition.x() and worldPosition.y() ....
	Z = Z.project(_viewMatrix * _modelMatrix, _projectionMatrix, getViewportFromPoint(start));
	QVector3D p1(start.x(), height() - start.y(), Z.z());
	QVector3D O = p1.unproject(_viewMatrix * _modelMatrix, _projectionMatrix, getViewportFromPoint(start));
	QVector3D p2(end.x(), height() - end.y(), Z.z());
	QVector3D P = p2.unproject(_viewMatrix * _modelMatrix, _projectionMatrix, getViewportFromPoint(start));
	QVector3D OP = P - O;
	return OP;
}

unsigned int GLWidget::loadTextureFromFile(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	//std::cout << "GLWidget::loadTextureFromFile : textureID = " << textureID << std::endl;

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = GL_RGBA;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 2)
			format = GL_RG;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

#include <chrono>
using namespace std::chrono;
int GLWidget::clickSelect(const QPoint& pixel)
{
	int id = -1;
	_selectedIDs.clear();

	//if (!_displayedObjectsIds.size())
	if (_visibleSwapped)
	{
		if (!_hiddenObjectsIds.size())
		{
			emit singleSelectionDone(id);
			return id;
		}
	}
	else
	{
		if (!_displayedObjectsIds.size())
		{
			emit singleSelectionDone(id);
			return id;
		}
	}

	QVector3D rayPos, rayDir;
	QVector3D intersectionPoint;
	QRect viewport = getViewportFromPoint(pixel);
	GLCamera* camera = _primaryCamera;
	if (_multiViewActive)
	{
		if (viewport.x() == viewport.width() && viewport.y() == 0)
		{
			camera = _primaryCamera;
		}
		else
		{
			camera = _orthoViewsCamera;
		}
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	convertClickToRay(pixel, viewport, camera, rayPos, rayDir);
	rayDir.normalize();

	// Get starting timepoint
    //auto start = high_resolution_clock::now();

	QMap<int, float> selectedIdsDist;
	for (int i : (_visibleSwapped ? _hiddenObjectsIds : _displayedObjectsIds))
	{
		TriangleMesh* mesh = _meshStore.at(i);
		if (mesh->getBoundingSphere().intersectsWithRay(rayPos, rayDir))
		{
			bool intersects = mesh->intersectsWithRay(rayPos, rayDir, intersectionPoint);
			//qDebug() << intPoint;
			if (intersects)
			{
				//id = i;
				selectedIdsDist[i] = intersectionPoint.distanceToPoint(rayPos);
				_selectedIDs.push_back(i);
				//_selectRect->setGeometry(_boundingRect);
				//_selectRect->setGeometry(mesh->getBoundingBox().project(_modelViewMatrix, _projectionMatrix, viewport, geometry()));
				//_selectRect->setGeometry(mesh->projectedRect(_modelViewMatrix, _projectionMatrix, viewport, geometry()));
				//_selectRect->show();
				//break;
			}
		}
		//else
		//_selectRect->hide();
	}
	//qDebug() << selectedIdsDist;
	if (!selectedIdsDist.isEmpty())
	{
		QMapIterator<int, float> it(selectedIdsDist);
		float lowestDist = std::numeric_limits<float>::max();
		while (it.hasNext())
		{
			it.next();
			float val = it.value();
			if (val < lowestDist)
				lowestDist = val;
		}
		id = selectedIdsDist.key(lowestDist);
	}
	qDebug() << "Selected Id: " << id;

	int colId = processSelection(pixel);

	qDebug() << "Color Id: " << colId;	

    //if (colId > 0)
        //id = colId;

	// Get ending timepoint
    //auto stop = high_resolution_clock::now();

	// Get duration. Substart timepoints to
	// get durarion. To cast it to proper unit
	// use duration cast method
    //auto duration = duration_cast<microseconds>(stop - start);

	//cout << "Time taken by function: " << duration.count() << " microseconds" << endl;

	QApplication::restoreOverrideCursor();

	emit singleSelectionDone(id);
	return id;
}

QList<int> GLWidget::sweepSelect(const QPoint& pixel)
{
	_selectedIDs.clear();

	//if (!_displayedObjectsIds.size())
	if (_visibleSwapped)
	{
		if (!_hiddenObjectsIds.size())
		{
			return _selectedIDs;
		}
	}
	else
	{
		if (!_displayedObjectsIds.size())
		{
			return _selectedIDs;
		}
	}

	QRect rubberRect = _rubberBand->geometry();
	if (rubberRect.width() == 0 || rubberRect.height() == 0)
	{
		return _selectedIDs;
	}

	QRect viewport = getViewportFromPoint(pixel);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	for (int i : (_visibleSwapped ? _hiddenObjectsIds : _displayedObjectsIds))
	{
		TriangleMesh* mesh = _meshStore.at(i);
		QRect objRect = mesh->getBoundingBox().project(_viewMatrix * _modelMatrix, _projectionMatrix, viewport, geometry());
		QRect interRect = rubberRect.intersected(objRect);
		// Intersection rectangle of rubberband and object is more than 5% of objRect
		bool intersects = (float(interRect.width() * interRect.height()) >= float(objRect.width() * objRect.height()) * 0.05f);

		if (intersects)
		{
			_selectedIDs.push_back(i);
		}
	}
	QApplication::restoreOverrideCursor();

	emit sweepSelectionDone(_selectedIDs);
	return _selectedIDs;
}

unsigned int GLWidget::colorToIndex(const QColor& color)
{
	int alpha = color.alpha();
	int red = color.red();
	int green = color.green();
	int blue = color.blue();
	unsigned int index =  ((alpha << 24) | (red << 16) | (green << 8) | (blue));
	return index;
}

QColor GLWidget::indexToColor(const unsigned int& index)
{
	int red = ((index >> 16) & 0xFF);
	int green = ((index >> 8) & 0xFF);
	int blue = (index & 0xFF);
	int alpha = ((index >> 24) & 0xFF);
	return QColor(red, green, blue, alpha);
}

void GLWidget::setView(QVector3D viewPos, QVector3D viewDir, QVector3D upDir, QVector3D rightDir)
{
	_primaryCamera->setView(viewPos, viewDir, upDir, rightDir);
	emit viewSet();
}

void GLWidget::setRotations(float xRot, float yRot, float zRot)
{
	// Rotation
	QQuaternion targetRotation = QQuaternion::fromEulerAngles(yRot, zRot, xRot); //Pitch, Yaw, Roll
	QQuaternion curRot = QQuaternion::slerp(_currentRotation, targetRotation, _slerpStep += _slerpFrac);

	// Translation
	QVector3D curPos = _currentTranslation - (_slerpStep * _currentTranslation) + (_boundingSphere.getCenter() * _slerpStep);

	// Set camera vectors
	QMatrix4x4 rotMat = QMatrix4x4(curRot.toRotationMatrix());
	QVector3D viewDir = -rotMat.row(2).toVector3D();
	QVector3D upDir = rotMat.row(1).toVector3D();
	QVector3D rightDir = rotMat.row(0).toVector3D();
	_primaryCamera->setView(curPos, viewDir, upDir, rightDir);

	// Set zoom
	float scaleStep = (_currentViewRange - _viewBoundingSphereDia) * _slerpFrac;
	_viewRange -= scaleStep;

	if (qFuzzyCompare(_slerpStep, 1.0f))
	{
		// Set camera vectors
		QMatrix4x4 rotMat = QMatrix4x4(curRot.toRotationMatrix());
		QVector3D viewDir = -rotMat.row(2).toVector3D();
		QVector3D upDir = rotMat.row(1).toVector3D();
		QVector3D rightDir = rotMat.row(0).toVector3D();
		_primaryCamera->setView(curPos, viewDir, upDir, rightDir);

		// Set all defaults
		_currentRotation = QQuaternion::fromRotationMatrix(_primaryCamera->getViewMatrix().toGenericMatrix<3, 3>());
		_currentTranslation = _primaryCamera->getPosition();
		_currentViewRange = _viewRange;
		_slerpStep = 0.0f;

		emit rotationsSet();
	}
}

void GLWidget::setZoomAndPan(float zoom, QVector3D pan)
{
	_slerpStep += _slerpFrac;

	// Translation
	QVector3D curPos = pan * _slerpFrac;
	_primaryCamera->move(curPos.x(), curPos.y(), curPos.z());

	// Set zoom
	float scaleStep = (_currentViewRange - zoom) * _slerpFrac;
	_viewRange -= scaleStep;

	if (qFuzzyCompare(_slerpStep, 1.0f))
	{
		// Set all defaults
		_currentTranslation = _primaryCamera->getPosition();
		_currentViewRange = _viewRange;
		_slerpStep = 0.0f;

		emit zoomAndPanSet();
	}
}

void GLWidget::closeEvent(QCloseEvent* event)
{
	if (_sphericalHarmonicsEditor)
	{
		_sphericalHarmonicsEditor->hide();
		_sphericalHarmonicsEditor->close();
	}

	if (_graysKleinEditor)
	{
		_graysKleinEditor->hide();
		_graysKleinEditor->close();
	}

	if (_superToroidEditor)
	{
		_superToroidEditor->hide();
		_superToroidEditor->close();
	}

	if (_superEllipsoidEditor)
	{
		_superEllipsoidEditor->hide();
		_superEllipsoidEditor->close();
	}

	if (_springEditor)
	{
		_springEditor->hide();
		_springEditor->close();
	}
	event->accept();
}

bool GLWidget::areLightsShown() const
{
	return _showLights;
}

void GLWidget::showLights(bool showLights)
{
	_showLights = showLights;
	update();
}

float GLWidget::getScreenGamma() const
{
	return _screenGamma;
}

void GLWidget::setScreenGamma(double screenGamma)
{
	_screenGamma = static_cast<float>(screenGamma);
	update();
}

bool GLWidget::getGammaCorrection() const
{
	return _gammaCorrection;
}

void GLWidget::enableGammaCorrection(bool gammaCorrection)
{
	_gammaCorrection = gammaCorrection;
	update();
}

bool GLWidget::getHdrToneMapping() const
{
	return _hdrToneMapping;
}

void GLWidget::enableHDRToneMapping(bool hdrToneMapping)
{
	_hdrToneMapping = hdrToneMapping;
	update();
}

RenderingMode GLWidget::getRenderingMode() const
{
	return _renderingMode;
}

void GLWidget::setRenderingMode(const RenderingMode& renderingMode)
{
	_renderingMode = renderingMode;
	update();
}

void GLWidget::setFloorTexRepeatT(double floorTexRepeatT)
{
	_floorTexRepeatT = static_cast<float>(floorTexRepeatT);
	updateFloorPlane();
	update();
}

void GLWidget::setFloorTexRepeatS(double floorTexRepeatS)
{
	_floorTexRepeatS = static_cast<float>(floorTexRepeatS);
	updateFloorPlane();
	update();
}

void GLWidget::setFloorOffsetPercent(double value)
{
	_floorOffsetPercent = static_cast<float>(value / 100.0f);
	updateFloorPlane();
	update();
}

void GLWidget::setSkyBoxFOV(double fov)
{
	_skyBoxFOV = static_cast<float>(fov);
	update();
}

void GLWidget::setSkyBoxTextureHDRI(bool hdrSet)
{
	_skyBoxTextureHDRI = hdrSet;
	update();
}

QColor GLWidget::getBgBotColor() const
{
	return _bgBotColor;
}

void GLWidget::setBgBotColor(const QColor& bgBotColor)
{
	_bgBotColor = bgBotColor;
	update();
}

QColor GLWidget::getBgTopColor() const
{
	return _bgTopColor;
}

void GLWidget::setBgTopColor(const QColor& bgTopColor)
{
	_bgTopColor = bgTopColor;
	update();
}

BoundingSphere GLWidget::getBoundingSphere() const
{
	return _boundingSphere;
}

std::vector<int> GLWidget::getDisplayedObjectsIds() const
{
	return _displayedObjectsIds;
}

bool GLWidget::isVisibleSwapped() const
{
	return _visibleSwapped;
}

void GLWidget::setShowFaceNormals(bool showFaceNormals)
{
	_showFaceNormals = showFaceNormals;
}

bool GLWidget::isFaceNormalsShown() const
{
	return _showFaceNormals;
}

bool GLWidget::isVertexNormalsShown() const
{
	return _showVertexNormals;
}

void GLWidget::setShowVertexNormals(bool showVertexNormals)
{
	_showVertexNormals = showVertexNormals;
}

bool GLWidget::isShaded() const
{
	return _displayMode == DisplayMode::SHADED;
}

void GLWidget::setDisplayMode(DisplayMode mode)
{
	_displayMode = mode;
}

float GLWidget::getZScale() const
{
	return _zScale;
}

void GLWidget::setZScale(const float& zScale)
{
	_zScale = zScale;
}

float GLWidget::getYScale() const
{
	return _yScale;
}

void GLWidget::setYScale(const float& yScale)
{
	_yScale = yScale;
}

float GLWidget::getXScale() const
{
	return _xScale;
}

void GLWidget::setXScale(const float& xScale)
{
	_xScale = xScale;
}

float GLWidget::getZRot() const
{
	return _zRot;
}

void GLWidget::setZRot(const float& zRot)
{
	_zRot = zRot;
}

float GLWidget::getYRot() const
{
	return _yRot;
}

void GLWidget::setYRot(const float& yRot)
{
	_yRot = yRot;
}

float GLWidget::getXRot() const
{
	return _xRot;
}

void GLWidget::setXRot(const float& xRot)
{
	_xRot = xRot;
}

float GLWidget::getZTran() const
{
	return _zTran;
}

void GLWidget::setZTran(const float& zTran)
{
	_zTran = zTran;
}

float GLWidget::getYTran() const
{
	return _yTran;
}

void GLWidget::setYTran(const float& yTran)
{
	_yTran = yTran;
}

float GLWidget::getXTran() const
{
	return _xTran;
}

void GLWidget::setXTran(const float& xTran)
{
	_xTran = xTran;
}

float GLWidget::highestModelZ()
{
	float highestZ = std::numeric_limits<float>::min();
	for (int i : (_visibleSwapped ? _hiddenObjectsIds : _displayedObjectsIds))
	{
		try
		{
			TriangleMesh* mesh = _meshStore.at(i);
			float z = mesh->getHighestZValue();
			if (z > highestZ)
				highestZ = z;
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception raised in GLWidget::highestModelZ\n" << ex.what() << std::endl;
			highestZ = _boundingSphere.getRadius();
		}
	}
	return highestZ;
}

float GLWidget::lowestModelZ()
{
	float lowestZ = std::numeric_limits<float>::max();
	for (int i : (_visibleSwapped ? _hiddenObjectsIds : _displayedObjectsIds))
	{
		try
		{
			TriangleMesh* mesh = _meshStore.at(i);
			float z = mesh->getLowestZValue();
			if (z < lowestZ)
				lowestZ = z;
		}
		catch (const std::exception& ex)
		{
			std::cout << "Exception raised in GLWidget::lowestModelZ\n" << ex.what() << std::endl;
			lowestZ = -_boundingSphere.getRadius();
		}
	}
	return lowestZ;
}

void GLWidget::showContextMenu(const QPoint& pos)
{
	if (QApplication::keyboardModifiers() != Qt::ControlModifier)
	{
		// Create menu and insert some actions
		QMenu myMenu;
		QListWidget* listWidgetModel = _viewer->getListModel();
		if (listWidgetModel->selectedItems().count() != 0 &&
			(_visibleSwapped ? _hiddenObjectsIds.size() != 0 : _displayedObjectsIds.size() != 0))
		{
			myMenu.addAction("Center Screen", _viewer, SLOT(centerScreen()));
			QList<QListWidgetItem*> selectedItems = listWidgetModel->selectedItems();
			if (selectedItems.count() <= 1 && selectedItems.at(0)->checkState() == Qt::Checked)
			{
				myMenu.addAction("Center Object List", this, SLOT(centerDisplayList()));
			}
			myMenu.addAction("Visualization Settings", _viewer, SLOT(showVisualizationModelPage()));
			myMenu.addAction("Transformations", _viewer, SLOT(showTransformationsPage()));
			if (_visibleSwapped)
				myMenu.addAction("Show", _viewer, SLOT(showSelectedItems()));
			else
				myMenu.addAction("Hide", _viewer, SLOT(hideSelectedItems()));
			if (_displayedObjectsIds.size() > 1)
				myMenu.addAction("Show Only", _viewer, SLOT(showOnlySelectedItems()));
			myMenu.addAction("Duplicate", _viewer, SLOT(duplicateSelectedItems()));
			myMenu.addAction("Delete", _viewer, SLOT(deleteSelectedItems()));
			myMenu.addAction("Mesh Info", _viewer, SLOT(displaySelectedMeshInfo()));
		}
		else
		{
			QAction* action = nullptr;
			if ((!_visibleSwapped && _displayedObjectsIds.size() != 0) || (_visibleSwapped && _hiddenObjectsIds.size() != 0))
			{
				myMenu.addAction(QIcon(":/new/prefix1/res/fit-all.png"), "Fit All", this, SLOT(fitAll()));

				action = myMenu.addAction(QIcon(":/new/prefix1/res/window-zoom.png"), "Zoom Area");
				action->setCheckable(true);
				connect(action, SIGNAL(triggered(bool)), _viewer, SLOT(on_toolButtonWindowZoom_clicked(bool)));
				myMenu.addAction(QIcon(":/new/prefix1/res/zoomview.png"), "Zoom", _viewer, SLOT(on_toolButtonZoomView_clicked()));
				myMenu.addAction(QIcon(":/new/prefix1/res/panview.png"), "Pan", _viewer, SLOT(on_toolButtonPanView_clicked()));
				myMenu.addAction(QIcon(":/new/prefix1/res/rotateview.png"), "Rotate", _viewer, SLOT(on_toolButtonRotateView_clicked()));
			}
			myMenu.addSeparator();

			if (_hiddenObjectsIds.size() != 0)
			{
				action = myMenu.addAction(QIcon(":/new/prefix1/res/showall.png"), "Show All", _viewer, SLOT(showAllItems()));
				action->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_A));
			}
			if (_displayedObjectsIds.size() != 0)
			{
				action = myMenu.addAction(QIcon(":/new/prefix1/res/hideall.png"), "Hide All", _viewer, SLOT(hideAllItems()));
				action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_A));
			}
			if (_hiddenObjectsIds.size() != 0)
			{
				action = myMenu.addAction(QIcon(":/new/prefix1/res/swapvisible.png"), "Swap Visible");
				action->setCheckable(true);
				action->setShortcut(QKeySequence(Qt::ALT + Qt::Key_S));
				action->setChecked(_visibleSwapped);
				connect(action, SIGNAL(triggered(bool)), _viewer, SLOT(on_toolButtonSwapVisible_clicked(bool)));
			}
			myMenu.addSeparator();
			myMenu.addAction("Background Color", this, SLOT(setBackgroundColor()));
		}
		// Show context menu at handling position
		myMenu.exec(mapToGlobal(pos));
	}
}

void GLWidget::centerDisplayList()
{
	QListWidget* listWidgetModel = _viewer->getListModel();
	if (listWidgetModel)
	{
		listWidgetModel->scrollToItem(listWidgetModel->selectedItems().at(0));
	}
}

#include "BackgroundColor.h"
void GLWidget::setBackgroundColor()
{
	BackgroundColor bgCol(this);
	bgCol.exec();
}
