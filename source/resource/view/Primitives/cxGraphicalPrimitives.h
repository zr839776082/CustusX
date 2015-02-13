/*=========================================================================
This file is part of CustusX, an Image Guided Therapy Application.

Copyright (c) 2008-2014, SINTEF Department of Medical Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
   may be used to endorse or promote products derived from this software 
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=========================================================================*/

#ifndef CXGRAPHICALPRIMITIVES_H_
#define CXGRAPHICALPRIMITIVES_H_

#include "cxResourceVisualizationExport.h"

#include <boost/shared_ptr.hpp>
#include "vtkForwardDeclarations.h"
#include "cxTransform3D.h"
#include "cxViewportListener.h"
#include <vtkPolyDataAlgorithm.h>

class QColor;
typedef vtkSmartPointer<class vtkPolyDataAlgorithm> vtkPolyDataAlgorithmPtr;
typedef vtkSmartPointer<class vtkArcSource> vtkArcSourcePtr;
typedef vtkSmartPointer<class vtkArrowSource> vtkArrowSourcePtr;

namespace cx
{

/**
 * \addtogroup cx_resource_visualization
 * @{
 */

/** \brief Helper for rendering a a polydata in 3D
 */
class cxResourceVisualization_EXPORT GraphicalPolyData3D
{
public:
	GraphicalPolyData3D(
			vtkPolyDataAlgorithmPtr source = vtkPolyDataAlgorithmPtr(),
			vtkRendererPtr renderer = vtkRendererPtr());
	void setRenderer(vtkRendererPtr renderer = vtkRendererPtr());
	void setSource(vtkPolyDataAlgorithmPtr source);
	void setData(vtkPolyDataPtr data);
	~GraphicalPolyData3D();
	void setColor(Vector3D color);
	void setPosition(Vector3D point);
	Vector3D getPosition() const;

	vtkActorPtr getActor();
	vtkPolyDataPtr getPolyData();
	vtkPolyDataAlgorithmPtr getSource();

private:
	vtkPolyDataAlgorithmPtr mSource;
	vtkPolyDataPtr mData;
	vtkPolyDataMapperPtr mMapper;
	vtkActorPtr mActor;
	vtkRendererPtr mRenderer;
};
typedef boost::shared_ptr<GraphicalPolyData3D> GraphicalPolyData3DPtr;


/** \brief Helper for rendering a point in 3D
 */
class cxResourceVisualization_EXPORT GraphicalPoint3D
{
	public:
		GraphicalPoint3D(vtkRendererPtr renderer = vtkRendererPtr());
    void setRenderer(vtkRendererPtr renderer = vtkRendererPtr());
		~GraphicalPoint3D();
		void setRadius(double radius);
//		void setColor(Vector3D color);
		void setColor(QColor color);
		void setValue(Vector3D point);
		Vector3D getValue() const;
		vtkActorPtr getActor();
		vtkPolyDataPtr getPolyData();

	private:
		vtkSphereSourcePtr source;
		vtkPolyDataMapperPtr mapper;
		vtkActorPtr actor;
		vtkRendererPtr mRenderer;
};
typedef boost::shared_ptr<GraphicalPoint3D> GraphicalPoint3DPtr;

/** \brief Helper for rendering a line in 3D
 */
class cxResourceVisualization_EXPORT GraphicalLine3D
{
	public:
		GraphicalLine3D(vtkRendererPtr renderer = vtkRendererPtr());
    void setRenderer(vtkRendererPtr renderer = vtkRendererPtr());
		~GraphicalLine3D();
		void setColor(QColor color);
		void setValue(Vector3D point1, Vector3D point2);
		void setStipple(int stipple);
		vtkActorPtr getActor();

	private:
		vtkPolyDataMapperPtr mapper;
		vtkActorPtr actor;
		vtkRendererPtr mRenderer;
		vtkLineSourcePtr source;
};
typedef boost::shared_ptr<GraphicalLine3D> GraphicalLine3DPtr;

/** \brief Helper for rendering a line in 3D
 */
class cxResourceVisualization_EXPORT GraphicalArc3D
{
  public:
  GraphicalArc3D(vtkRendererPtr renderer = vtkRendererPtr());
    ~GraphicalArc3D();
	void setColor(QColor color);
    void setValue(Vector3D point1, Vector3D point2, Vector3D center);
    void setStipple(int stipple);
    vtkActorPtr getActor();

  private:
    vtkPolyDataMapperPtr mapper;
    vtkActorPtr actor;
    vtkRendererPtr mRenderer;
    vtkArcSourcePtr source;
};
typedef boost::shared_ptr<GraphicalArc3D> GraphicalArc3DPtr;

/** \brief Helper for rendering an arrow in 3D
 */
class cxResourceVisualization_EXPORT GraphicalArrow3D
{
  public:
	GraphicalArrow3D(vtkRendererPtr renderer = vtkRendererPtr());
    ~GraphicalArrow3D();
	void setColor(QColor color);
    void setValue(Vector3D base, Vector3D normal, double length);

  private:
    vtkPolyDataMapperPtr mapper;
    vtkActorPtr actor;
    vtkRendererPtr mRenderer;
    vtkArrowSourcePtr source;
};
typedef boost::shared_ptr<GraphicalArrow3D> GraphicalArrow3DPtr;

/**\brief Helper for drawing a rectangle in 3D.
 */
class cxResourceVisualization_EXPORT Rect3D
{
public:
  Rect3D(vtkRendererPtr renderer, QColor color);
  ~Rect3D();
  void setColor(QColor color);
  void updatePosition(const DoubleBoundingBox3D bb, const Transform3D& M);
  void setLine(bool on, int width);
  void setSurface(bool on);

private:
  vtkPolyDataMapperPtr mapper;
  vtkActorPtr actor;
  vtkRendererPtr mRenderer;
  vtkPolyDataPtr mPolyData;
  vtkPointsPtr mPoints;
  vtkCellArrayPtr mSide;
};
typedef boost::shared_ptr<class Rect3D> Rect3DPtr;

/** \brief Helper for rendering 3D text that faces the camera and
 *  has a constant viewed size.
 */
class cxResourceVisualization_EXPORT FollowerText3D
{
  public:
	FollowerText3D(vtkRendererPtr renderer = vtkRendererPtr());
  void setRenderer(vtkRendererPtr renderer = vtkRendererPtr());

    ~FollowerText3D();
	void setColor(QColor color);
    void setText(QString text);
    void setPosition(Vector3D pos);

    void setSize(double val);
    void setSizeInNormalizedViewport(bool on, double size);
    vtkFollowerPtr getActor();

    void scaleText(); ///< internal method

  private:
    vtkVectorTextPtr mText;
    vtkFollowerPtr mFollower;
    vtkRendererPtr mRenderer;
    double mSize;

    ViewportListenerPtr mViewportListener;
};
typedef boost::shared_ptr<FollowerText3D> FollowerText3DPtr;

/** \brief Helper for rendering 3D text that faces the camera and
 *  has a constant viewed size, always on top.
 */
class cxResourceVisualization_EXPORT CaptionText3D
{
public:
	CaptionText3D(vtkRendererPtr renderer = vtkRendererPtr());
	void setRenderer(vtkRendererPtr renderer = vtkRendererPtr());

	~CaptionText3D();
	void setColor(QColor color);
	void setText(QString text);
	void setPosition(Vector3D pos);

	void setSize(double val);
	//    void setSizeInNormalizedViewport(bool on, double size);
	vtkCaptionActor2DPtr getActor();

	//    void scaleText(); ///< internal method

private:
	vtkCaptionActor2DPtr mText;
	//    vtkFollowerPtr mFollower;
	vtkRendererPtr mRenderer;

	//    ViewportListenerPtr mViewportListener;
};
typedef boost::shared_ptr<CaptionText3D> CaptionText3DPtr;

/**
 * @}
 */

}

#endif /*CXGRAPHICALPRIMITIVES_H_*/