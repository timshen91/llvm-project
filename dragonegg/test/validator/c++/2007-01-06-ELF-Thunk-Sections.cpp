// RUN: %dragonegg %s -S -o - | FileCheck %s
// PR1085
// XFAIL: gcc-4.7

// CHECK: define weak void @_ZThn8_N
// CHECK: define weak void @_ZThn8_N
// CHECK: define weak void @_ZThn8_N
// CHECK: define weak void @_ZThn8_N
// CHECK: define weak void @_ZThn8_N
// CHECK: define weak void @_ZThn8_N

class 
__attribute__((visibility("default"))) QGenericArgument
{
	public:inline QGenericArgument(const char *aName = 0, const void *aData = 0):_data(aData), _name(aName) {
	}
	private:const void *_data;
	const char     *_name;
};
struct __attribute__ ((
		       visibility("default"))) QMetaObject
{
	struct {
	}
	                d;
};
class 
__attribute__((visibility("default"))) QObject
{
	virtual const QMetaObject *metaObject() const;
};
class 
__attribute__((visibility("default"))) QPaintDevice
{
	public:enum PaintDeviceMetric {
		PdmWidth = 1, PdmHeight, PdmWidthMM, PdmHeightMM, PdmNumColors, PdmDepth, PdmDpiX, PdmDpiY, PdmPhysicalDpiX, PdmPhysicalDpiY
	};
	virtual ~ QPaintDevice();
	union {
	}
	                ct;
};
class 
__attribute__((visibility("default"))) QWidget:public QObject, public QPaintDevice
{
};
class 
__attribute__((visibility("default"))) QDialog:public QWidget
{
};
class           TopicChooser:public QDialog {
	virtual const QMetaObject *metaObject() const;
};
const QMetaObject *TopicChooser::
metaObject() const
{
}
