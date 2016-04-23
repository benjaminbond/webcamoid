/* Webcamoid, webcam capture application.
 * Copyright (C) 2011-2016  Gonzalo Exequiel Pedone
 *
 * Webcamoid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Webcamoid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Webcamoid. If not, see <http://www.gnu.org/licenses/>.
 *
 * Web-Site: http://webcamoid.github.io/
 */

#ifndef CAPTURE_H
#define CAPTURE_H

#include <sys/time.h>
#include <QSize>
#include <QMutex>
#include <QWaitCondition>

#include <ak.h>

#include "framegrabber.h"

DEFINE_GUID(CLSID_SampleGrabber, 0xc1f400a0, 0x3f08, 0x11d3, 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37);
DEFINE_GUID(CLSID_NullRenderer, 0xc1f400a4, 0x3f08, 0x11d3, 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37);
static const GUID GUID_DEVINTERFACE_USB_DEVICE = {0xA5DCBF10, 0x6530, 0x11D2, {0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED}};

Q_CORE_EXPORT HINSTANCE qWinAppInst();

typedef QSharedPointer<IGraphBuilder> GraphBuilderPtr;
typedef QSharedPointer<IBaseFilter> BaseFilterPtr;
typedef QSharedPointer<ISampleGrabber> SampleGrabberPtr;
typedef QSharedPointer<IAMStreamConfig> StreamConfigPtr;
typedef QSharedPointer<FrameGrabber> FrameGrabberPtr;
typedef QSharedPointer<IMoniker> MonikerPtr;
typedef QMap<QString, MonikerPtr> MonikersMap;
typedef QSharedPointer<AM_MEDIA_TYPE> MediaTypePtr;
typedef QList<MediaTypePtr> MediaTypesList;
typedef QSharedPointer<IPin> PinPtr;
typedef QList<PinPtr> PinList;

__inline bool operator <(REFGUID guid1, REFGUID guid2)
{
    return guid1.Data1 < guid2.Data1;
}

class Capture: public QObject
{
    Q_OBJECT
    Q_ENUMS(IoMethod)
    Q_PROPERTY(QStringList webcams
               READ webcams
               NOTIFY webcamsChanged)
    Q_PROPERTY(QString device
               READ device
               WRITE setDevice
               RESET resetDevice
               NOTIFY deviceChanged)
    Q_PROPERTY(QString ioMethod
               READ ioMethod
               WRITE setIoMethod
               RESET resetIoMethod
               NOTIFY ioMethodChanged)
    Q_PROPERTY(int nBuffers
               READ nBuffers
               WRITE setNBuffers
               RESET resetNBuffers)

    public:
        enum IoMethod
        {
            IoMethodUnknown = -1,
            IoMethodDirectRead,
            IoMethodGrabSample,
            IoMethodGrabBuffer
        };

        explicit Capture();

        Q_INVOKABLE QStringList webcams() const;
        Q_INVOKABLE QString device() const;
        Q_INVOKABLE QList<int> streams() const;
        Q_INVOKABLE QList<int> listTracks(const QString &mimeType);
        Q_INVOKABLE QString ioMethod() const;
        Q_INVOKABLE int nBuffers() const;
        Q_INVOKABLE QString description(const QString &webcam) const;
        Q_INVOKABLE QVariantList caps(const QString &webcam) const;
        Q_INVOKABLE QString capsDescription(const AkCaps &caps) const;
        Q_INVOKABLE QVariantList imageControls() const;
        Q_INVOKABLE bool setImageControls(const QVariantMap &imageControls) const;
        Q_INVOKABLE bool resetImageControls() const;
        Q_INVOKABLE QVariantList cameraControls() const;
        Q_INVOKABLE bool setCameraControls(const QVariantMap &cameraControls) const;
        Q_INVOKABLE bool resetCameraControls() const;
        Q_INVOKABLE AkPacket readFrame();

    private:
        QStringList m_webcams;
        QString m_device;
        QList<int> m_streams;
        qint64 m_id;
        AkCaps m_caps;
        AkFrac m_timeBase;
        IoMethod m_ioMethod;
        QMap<QString, QSize> m_resolution;
        IGraphBuilder *m_graph;
        SampleGrabberPtr m_grabber;
        FrameGrabber m_frameGrabber;
        QByteArray m_curBuffer;
        QMutex m_mutex;
        QWaitCondition m_waitCondition;

        AkCaps capsFromMediaType(const MediaTypePtr &mediaType) const;
        HRESULT enumerateCameras(IEnumMoniker **ppEnum) const;
        MonikersMap listMonikers() const;
        MonikerPtr findMoniker(const QString &webcam) const;
        IBaseFilter *findFilterP(const QString &webcam) const;
        BaseFilterPtr findFilter(const QString &webcam) const;
        MediaTypesList listMediaTypes(const QString &webcam) const;
        MediaTypesList listMediaTypes(IBaseFilter *filter) const;
        bool isPinConnected(IPin *pPin, bool *ok=NULL) const;
        PinPtr findUnconnectedPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir) const;
        bool connectFilters(IGraphBuilder *pGraph, IBaseFilter *pSrc, IBaseFilter *pDest) const;
        PinList enumPins(IBaseFilter *filter, PIN_DIRECTION direction) const;
        bool createDeviceNotifier();
        static LRESULT CALLBACK deviceEvents(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        static void deleteUnknown(IUnknown *unknown);
        static void deleteMediaType(AM_MEDIA_TYPE *mediaType);
        static void deletePin(IPin *pin);

    signals:
        void webcamsChanged(const QStringList &webcams) const;
        void deviceChanged(const QString &device);
        void streamsChanged(const QList<int> &streams);
        void ioMethodChanged(const QString &ioMethod);
        void nBuffersChanged(int nBuffers);
        void error(const QString &message);
        void imageControlsChanged(const QVariantMap &imageControls) const;
        void cameraControlsChanged(const QVariantMap &cameraControls) const;

    public slots:
        bool init();
        void uninit();
        void setDevice(const QString &device);
        void setStreams(const QList<int> &streams);
        void setIoMethod(const QString &ioMethod);
        void setNBuffers(int nBuffers);
        void resetDevice();
        void resetStreams();
        void resetIoMethod();
        void resetNBuffers();
        void reset();

    private slots:
        void frameReceived(qreal time, const QByteArray &buffer);
};

#endif // CAPTURE_H