#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QtGui>
#include <QtOpenGL>
#include <QOpenGLFunctions_4_5_Core>
#include <QImage>
#include <QColor>

#include <math.h>
#include "GLCamera.h"
#include "BoundingSphere.h"
#include "TriangleMesh.h"

/* Custom OpenGL Viewer Widget */

class TextRenderer;
class SphericalHarmonicsEditor;
class SuperToroidEditor;
class SuperEllipsoidEditor;
class SpringEditor;
class ClippingPlanesEditor;
class GraysKleinEditor;
class AssImpModelLoader;
class Plane;
class Cube;
class Cone;

class AssImpModelLoader;

class ModelViewer;

enum class ViewMode { TOP, BOTTOM, LEFT, RIGHT, FRONT, BACK, ISOMETRIC, DIMETRIC, TRIMETRIC, NONE };
enum class ViewProjection { ORTHOGRAPHIC, PERSPECTIVE };
enum class DisplayMode { SHADED, WIREFRAME, WIRESHADED, REALSHADED };
enum class RenderingMode { ADS_PHONG, PBR_DIRECT_LIGHTING, PBR_TEXTURED_LIGHTING };

class GLWidget : public QOpenGLWidget, QOpenGLFunctions_4_5_Core
{
	friend class ClippingPlanesEditor;
	Q_OBJECT
public:
	GLWidget(QWidget* parent = 0, const char* name = 0);
	~GLWidget();
	void updateView();

	void resizeView(int w, int h) { resizeGL(w, h); }
	void setViewMode(ViewMode mode);
	void setProjection(ViewProjection proj);

	void setMultiView(bool active) { _multiViewActive = active; }
	void setRotationActive(bool active);
	void setPanningActive(bool active);
	void setZoomingActive(bool active);

	void beginWindowZoom();
	void performWindowZoom();

	void setDisplayList(const std::vector<int>& ids);
	void duplicateObjects(const std::vector<int>& ids);

	void updateFloorPlane();
	void updateBoundingSphere();
	int getModelNum() const
	{
		return _modelNum;
	}

	void updateClippingPlane();
	void showClippingPlaneEditor(bool show);
	void showAxis(bool show);

	void showShadows(bool show);
	void showEnvironment(bool show);
	void showSkyBox(bool show);
	void showReflections(bool show);
	void showFloor(bool show);
	void showFloorTexture(bool show);
	void setFloorTexture(QImage img);

	std::vector<TriangleMesh*> getMeshStore() const { return _meshStore; }

	void addToDisplay(TriangleMesh*);
	void removeFromDisplay(int index);
	void centerScreen(std::vector<int> selectedIDs);
	void select(int id);
	void deselect(int id);

	bool loadAssImpModel(const QString& fileName, QString& error);

	void enableADSDiffuseTexMap(const std::vector<int>& ids, const bool& enable);
	void setADSDiffuseTexMap(const std::vector<int>& ids, const QString& path);
	void clearADSDiffuseTexMap(const std::vector<int>& ids);

	void enableADSSpecularTexMap(const std::vector<int>& ids, const bool& enable);
	void setADSSpecularTexMap(const std::vector<int>& ids, const QString& path);
	void clearADSSpecularTexMap(const std::vector<int>& ids);

	void enableADSEmissiveTexMap(const std::vector<int>& ids, const bool& enable);
	void setADSEmissiveTexMap(const std::vector<int>& ids, const QString& path);
	void clearADSEmissiveTexMap(const std::vector<int>& ids);

	void enableADSNormalTexMap(const std::vector<int>& ids, const bool& enable);
	void setADSNormalTexMap(const std::vector<int>& ids, const QString& path);
	void clearADSNormalTexMap(const std::vector<int>& ids);

	void enableADSHeightTexMap(const std::vector<int>& ids, const bool& enable);
	void setADSHeightTexMap(const std::vector<int>& ids, const QString& path);
	void clearADSHeightTexMap(const std::vector<int>& ids);

	void enableADSOpacityTexMap(const std::vector<int>& ids, const bool& enable);
	void invertADSOpacityTexMap(const std::vector<int>& ids, const bool& inverted);
	void setADSOpacityTexMap(const std::vector<int>& ids, const QString& path);
	void clearADSOpacityTexMap(const std::vector<int>& ids);

	void clearADSTexMaps(const std::vector<int>& ids);

	void setMaterialToObjects(const std::vector<int>& ids, const GLMaterial& mat);
	void setPBRAlbedoColor(const std::vector<int>& ids, const QColor& col);
	void setPBRMetallic(const std::vector<int>& ids, const float& val);
	void setPBRRoughness(const std::vector<int>& ids, const float& val);

	void clearPBRTexMaps(const std::vector<int>& ids);
	void enablePBRAlbedoTexMap(const std::vector<int>& ids, const bool& enable);
	void setPBRAlbedoTexMap(const std::vector<int>& ids, const QString& path);
	void clearPBRAlbedoTexMap(const std::vector<int>& ids);
	void enablePBRMetallicTexMap(const std::vector<int>& ids, const bool& enable);
	void setPBRMetallicTexMap(const std::vector<int>& ids, const QString& path);
	void clearPBRMetallicTexMap(const std::vector<int>& ids);
	void enablePBRRoughnessTexMap(const std::vector<int>& ids, const bool& enable);
	void setPBRRoughnessTexMap(const std::vector<int>& ids, const QString& path);
	void clearPBRRoughnessTexMap(const std::vector<int>& ids);
	void enablePBRNormalTexMap(const std::vector<int>& ids, const bool& enable);
	void setPBRNormalTexMap(const std::vector<int>& ids, const QString& path);
	void clearPBRNormalTexMap(const std::vector<int>& ids);
	void enablePBRAOTexMap(const std::vector<int>& ids, const bool& enable);
	void setPBRAOTexMap(const std::vector<int>& ids, const QString& path);
	void clearPBRAOTexMap(const std::vector<int>& ids);

	void enablePBROpacityTexMap(const std::vector<int>& ids, const bool& enable);
	void setPBROpacityTexMap(const std::vector<int>& ids, const QString& path);
	void invertPBROpacityTexMap(const std::vector<int>& ids, const bool& inverted);
	void clearPBROpacityTexMap(const std::vector<int>& ids);

	void enablePBRHeightTexMap(const std::vector<int>& ids, const bool& enable);
	void setPBRHeightTexMap(const std::vector<int>& ids, const QString& path);
	void clearPBRHeightTexMap(const std::vector<int>& ids);
	void setPBRHeightScale(const std::vector<int>& ids, const float& scale);
	void setTransformation(const std::vector<int>& ids, const QVector3D& trans, const QVector3D& rot, const QVector3D& scale);
	void resetTransformation(const std::vector<int>& ids);
	void setTexture(const std::vector<int>& ids, const QImage& texImage);
	void setSkyBoxTextureFolder(QString folder);

public:
	float getXTran() const;
	void setXTran(const float& xTran);

	float getYTran() const;
	void setYTran(const float& yTran);

	float getZTran() const;
	void setZTran(const float& zTran);

	float getXRot() const;
	void setXRot(const float& xRot);

	float getYRot() const;
	void setYRot(const float& yRot);

	float getZRot() const;
	void setZRot(const float& zRot);

	float getXScale() const;
	void setXScale(const float& xScale);

	float getYScale() const;
	void setYScale(const float& yScale);

	float getZScale() const;
	void setZScale(const float& zScale);

	QVector4D getAmbientLight() const;
	void setAmbientLight(const QVector4D& ambientLight);

	QVector4D getDiffuseLight() const;
	void setDiffuseLight(const QVector4D& diffuseLight);

	QVector4D getSpecularLight() const;
	void setSpecularLight(const QVector4D& specularLight);

	QVector3D getLightPosition() const;
	void setLightOffset(const QVector3D& offset);

	bool isShaded() const;
	void setDisplayMode(DisplayMode mode);

	bool isVertexNormalsShown() const;
	void setShowVertexNormals(bool showVertexNormals);

	bool isFaceNormalsShown() const;
	void setShowFaceNormals(bool showFaceNormals);

	std::vector<int> getDisplayedObjectsIds() const;

	bool isVisibleSwapped() const;

	BoundingSphere getBoundingSphere() const;

	QColor getBgTopColor() const;
	void setBgTopColor(const QColor& bgTopColor);

	QColor getBgBotColor() const;
	void setBgBotColor(const QColor& bgBotColor);

	RenderingMode getRenderingMode() const;
	void setRenderingMode(const RenderingMode& renderingMode);

	bool getHdrToneMapping() const;
	bool getGammaCorrection() const;
	float getScreenGamma() const;

	bool areLightsShown() const;

	void cleanUpShaders();

signals:
	void windowZoomEnded();
	void rotationsSet();
	void zoomAndPanSet();
	void viewSet();
	void displayListSet();
	void singleSelectionDone(int);
	void sweepSelectionDone(QList<int>);
	void floorShown(bool);
	void visibleSwapped(bool);
	void loadingAssImpModelCancelled();

public slots:
	void animateViewChange();
	void animateFitAll();
	void animateWindowZoom();
	void animateCenterScreen();
	void stopAnimations();
	void checkAndStopTimers();
	void fitAll();
	void setAutoFitViewOnUpdate(bool update);
	void performKeyboardNav();
	void disableLowRes();
	void lockLightAndCamera(bool lock);
	void setFloorTexRepeatS(double floorTexRepeatS);
	void setFloorTexRepeatT(double floorTexRepeatT);
	void setFloorOffsetPercent(double value);
	void setSkyBoxFOV(double fov);
	void setSkyBoxTextureHDRI(bool hdrSet);
	void enableHDRToneMapping(bool hdrToneMapping);
	void enableGammaCorrection(bool gammaCorrection);
	void setScreenGamma(double screenGamma);
	void showLights(bool showLights);
	void showFileReadingProgress(float percent);
	void showMeshLoadingProgress(float percent);
	void showModelLoadingProgress(int nodeNum, int totalNodes);
	void swapVisible(bool checked);
	void cancelAssImpModelLoading();

private slots:
	void showContextMenu(const QPoint& pos);
	void centerDisplayList();
	void setBackgroundColor();

protected:
	void initializeGL();
	void createCappingPlanes();
	void resizeGL(int width, int height);
	void paintGL();

	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void wheelEvent(QWheelEvent*);
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent(QKeyEvent* event);
	void closeEvent(QCloseEvent* event);

private:
	bool loadCompileAndLinkShaderFromFile(QOpenGLShaderProgram* prog, const QString& vertexProg,
		const QString& fragmentProg, const QString& geometryProg = "",
		const QString& tessControlProg = "", const QString& tessEvalProg = "");
	void createShaderPrograms();
	void createLights();
	void createGeometry();

	void loadEnvMap();
	void loadIrradianceMap();
	void loadFloor();

	void drawMesh(QOpenGLShaderProgram* prog);
	void drawSectionCapping();
	void drawFloor();
	void drawSkyBox();
	void drawVertexNormals();
	void drawFaceNormals();
	void drawAxis();
	void drawCornerAxis();
	void drawLights();

	void render(GLCamera* camera);
	void renderToShadowBuffer();
	int processSelection(const QPoint& pixel);
	void renderQuad();

	void gradientBackground(float top_r, float top_g, float top_b, float top_a,
		float bot_r, float bot_g, float bot_b, float bot_a);
	void splitScreen();

	void setRotations(float xRot, float yRot, float zRot);
	void setZoomAndPan(float zoom, QVector3D pan);
	void setView(QVector3D viewPos, QVector3D viewDir, QVector3D upDir, QVector3D rightDir);

	void convertClickToRay(const QPoint& pixel, const QRect& viewport, GLCamera* camera, QVector3D& orig, QVector3D& dir);
	int clickSelect(const QPoint& pixel);
	QList<int> sweepSelect(const QPoint& pixel);
	unsigned int colorToIndex(const QColor& color);
	QColor indexToColor(const unsigned int& index);

	float highestModelZ();
	float lowestModelZ();

	QRect getViewportFromPoint(const QPoint& pixel);
	QRect getClientRectFromPoint(const QPoint& pixel);
	QVector3D get3dTranslationVectorFromMousePoints(const QPoint& start, const QPoint& end);
	unsigned int loadTextureFromFile(const char* path);
	void setupClippingUniforms(QOpenGLShaderProgram* prog, QVector3D pos);

private:
	QMap<int, bool> _keys;
	DisplayMode _displayMode;
	RenderingMode _renderingMode;
	QColor      _bgTopColor;
	QColor      _bgBotColor;
	bool _windowZoomActive;
	bool _viewZooming;
	bool _viewPanning;
	bool _viewRotating;
	int _modelNum;
	QImage _texImage, _texBuffer, _floorTexImage;
	float _floorTexRepeatS, _floorTexRepeatT;
	TextRenderer* _textRenderer;
	TextRenderer* _axisTextRenderer;
	QString _modelName;

	QVector3D _currentTranslation;
	QQuaternion _currentRotation;
	float _slerpStep;
	float _slerpFrac;

	float _currentViewRange;
	float _scaleFrac;

	float _viewRange;
	float _viewBoundingSphereDia;
	float _FOV;

	bool _autoFitViewOnUpdate;

	QPoint _leftButtonPoint;
	QPoint _rightButtonPoint;
	QPoint _middleButtonPoint;

	QRubberBand* _rubberBand;
	QRubberBand* _selectRect;
	QVector3D _rubberBandPan;
	GLfloat _rubberBandZoomRatio;
	float _rubberBandRadius;
	QVector3D _rubberBandCenter;
	QList<int> _selectedIDs;
	unsigned int _selectionFBO;
	unsigned int _selectionRBO;
	unsigned int _selectionDBO;

	bool _multiViewActive;

	bool _showAxis;

	float _clipXCoeff;
	float _clipYCoeff;
	float _clipZCoeff;

	float _clipDX;
	float _clipDY;
	float _clipDZ;

	bool _clipYZEnabled;
	bool _clipZXEnabled;
	bool _clipXYEnabled;

	bool _clipXFlipped;
	bool _clipYFlipped;
	bool _clipZFlipped;

	bool _showVertexNormals;
	bool _showFaceNormals;

	bool _envMapEnabled;
	bool _shadowsEnabled;
	bool _reflectionsEnabled;
	bool _floorDisplayed;
	bool _floorTextureDisplayed;
	bool _skyBoxEnabled;

	bool _lowResEnabled;
	bool _lockLightAndCamera;

	unsigned int _shadowWidth;
	unsigned int _shadowHeight;

	float _xTran;
	float _yTran;
	float _zTran;

	float _xRot;
	float _yRot;
	float _zRot;

	float _xScale;
	float _yScale;
	float _zScale;

	QVector4D _ambientLight;
	QVector4D _diffuseLight;
	QVector4D _specularLight;

	QVector3D _lightPosition;
	float _lightOffsetX;
	float _lightOffsetY;
	float _lightOffsetZ;

	QMatrix4x4 _lightSpaceMatrix;

	QMatrix4x4 _projectionMatrix, _viewMatrix, _modelMatrix;
	QMatrix4x4 _modelViewMatrix;
	QMatrix4x4 _viewportMatrix;

	QOpenGLShaderProgram* _fgShader;
	QOpenGLShaderProgram* _axisShader;
	QOpenGLShaderProgram* _vertexNormalShader;
	QOpenGLShaderProgram* _faceNormalShader;
	QOpenGLShaderProgram* _shadowMappingShader;
	QOpenGLShaderProgram* _skyBoxShader;
	QOpenGLShaderProgram* _irradianceShader;
	QOpenGLShaderProgram* _prefilterShader;
	QOpenGLShaderProgram* _brdfShader;
	QOpenGLShaderProgram* _lightCubeShader;
	QOpenGLShaderProgram* _clippingPlaneShader;
	QOpenGLShaderProgram* _clippedMeshShader;
	QOpenGLShaderProgram* _selectionShader;

	unsigned int             _environmentMap;
	unsigned int             _shadowMap;
	unsigned int             _shadowMapFBO;
	unsigned int			 _irradianceMap;
	unsigned int             _prefilterMap;
	unsigned int             _brdfLUTTexture;
	float                    _floorSize;
	float					 _floorOffsetPercent;
	QVector3D                _floorCenter;

	QOpenGLShaderProgram* _textShader;

	QOpenGLShaderProgram* _bgShader;
	QOpenGLVertexArrayObject _bgVAO;

	QOpenGLShaderProgram* _bgSplitShader;
	QOpenGLVertexArrayObject _bgSplitVAO;
	QOpenGLBuffer _bgSplitVBO;

	QOpenGLVertexArrayObject _axisVAO;
	QOpenGLBuffer _axisVBO;
	QOpenGLBuffer _axisCBO;

	std::vector<TriangleMesh*> _meshStore;
	std::vector<int> _displayedObjectsIds;
	std::vector<int> _hiddenObjectsIds;
	std::vector<int> _centerScreenObjectIDs;
	bool _visibleSwapped;

	QVBoxLayout* _editorLayout;
	QFormLayout* _lowerLayout;
	QFormLayout* _upperLayout;

	SphericalHarmonicsEditor* _sphericalHarmonicsEditor;
	SuperToroidEditor* _superToroidEditor;
	SuperEllipsoidEditor* _superEllipsoidEditor;
	SpringEditor* _springEditor;
	GraysKleinEditor* _graysKleinEditor;
	ClippingPlanesEditor* _clippingPlanesEditor;
	Plane* _clippingPlaneXY;
	Plane* _clippingPlaneYZ;
	Plane* _clippingPlaneZX;
	bool _cappingEnabled;
	unsigned int _cappingTexture;

	ViewMode _viewMode;
	ViewProjection _projection;

	GLCamera* _primaryCamera;
	GLCamera* _orthoViewsCamera;

	QTimer* _keyboardNavTimer;
	QTimer* _animateViewTimer;
	QTimer* _animateFitAllTimer;
	QTimer* _animateWindowZoomTimer;
	QTimer* _animateCenterScreenTimer;

	BoundingSphere _boundingSphere;
	BoundingSphere _selectionBoundingSphere;
	Plane* _floorPlane;
	Cube* _skyBox;
	vector<QString> _skyBoxFaces;
	float _skyBoxFOV;
	bool  _skyBoxTextureHDRI;
	bool  _gammaCorrection;
	float _screenGamma;
	bool  _hdrToneMapping;

	Cone* _axisCone;

	Cube* _lightCube;
	bool _showLights;

	QOpenGLShaderProgram* _debugShader;

	ModelViewer* _viewer;

	unsigned int _quadVAO;
	unsigned int _quadVBO;

	unsigned long long _displayedObjectsMemSize;

    AssImpModelLoader* _assimpModelLoader;
};

#endif
