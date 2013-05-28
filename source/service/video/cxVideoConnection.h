// This file is part of CustusX, an Image Guided Therapy Application.
//
// Copyright (C) 2008- SINTEF Technology & Society, Medical Technology
//
// CustusX is fully owned by SINTEF Medical Technology (SMT). CustusX source
// code and binaries can only be used by SMT and those with explicit permission
// from SMT. CustusX shall not be distributed to anyone else.
//
// CustusX is a research tool. It is NOT intended for use or certified for use
// in a normal clinical setting. SMT does not take responsibility for its use
// in any way.
//
// See CustusX_License.txt for more information.
#ifndef CXOPENIGTLINKRTSOURCE_H_
#define CXOPENIGTLINKRTSOURCE_H_

#include <vector>
#include <map>
#include <boost/array.hpp>
#include "sscForwardDeclarations.h"
#include "sscVideoSource.h"
#include "sscProbeData.h"

class QTimer;
typedef vtkSmartPointer<class vtkImageImport> vtkImageImportPtr;
typedef vtkSmartPointer<class vtkImageAlgorithm> vtkImageAlgorithmPtr;

namespace cx
{
/**
 * \file
 * \addtogroup cxServiceVideo
 * @{
 */

typedef boost::shared_ptr<class ImageReceiverThread> ImageReceiverThreadPtr;
typedef boost::shared_ptr<class BasicVideoSource> BasicVideoSourcePtr;

/** \brief Represent one video grabber connection.
 *
 * Connect to a video/scanner interface and receive
 * all video and probe data from it. Convert to video
 * streams, set all data in probe if available.
 * Video Streams are also available directly from this
 * object.
 *
 * Refactored from old class OpenIGTLinkRTSource.
 *
 *  \ingroup cxServiceVideo
 *  \date Oct 31, 2010
 *  \date Feb 26, 2013
 *  \author Christian Askeland, SINTEF
 */
class VideoConnection : public QObject
{
Q_OBJECT
public:
	VideoConnection();
	virtual ~VideoConnection();
	virtual bool isConnected() const;

	void directLink(std::map<QString, QString> args);
	void connectServer(QString address, int port);
	void disconnectServer();

	std::vector<ssc::VideoSourcePtr> getVideoSources();

signals:
	bool connected(bool);
	void fps(int fps);
	void videoSourcesChanged();

private slots:
	void clientFinishedSlot();
	void imageReceivedSlot();
	void statusReceivedSlot();
	void fpsSlot(double fps);
	void connectedSlot(bool on);
	void connectVideoToProbe();
	void useUnusedProbeDataSlot();///< If no probe is available the ProbeData is saved and this slot is called when a probe becomes available

private:
	void updateImage(ssc::ImagePtr message); // called by receiving thread when new data arrives.
	void runClient(ImageReceiverThreadPtr client);
	void stopClient(); ///< Get rid of the mClient thread.
	void updateStatus(ssc::ProbeDataPtr message);
	void startAllSources();
	void removeSourceFromProbe(ssc::ToolPtr tool);

	ImageReceiverThreadPtr mClient;
	bool mConnected;
	double mFPS;
	std::vector<ssc::ProbeDataPtr> mUnsusedProbeDataVector;

	std::vector<BasicVideoSourcePtr> mSources;
};
typedef boost::shared_ptr<VideoConnection> VideoConnectionPtr;

/**
 * @}
 */
} // namespace cx

#endif /* CXOPENIGTLINKRTSOURCE_H_ */
