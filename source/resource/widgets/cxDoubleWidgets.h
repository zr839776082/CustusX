// This file is part of SSC,
// a C++ Library supporting Image Guided Therapy Applications.
//
// Copyright (C) 2008- SINTEF Medical Technology
// Copyright (C) 2008- Sonowand AS
//
// SSC is owned by SINTEF Medical Technology and Sonowand AS,
// hereafter named the owners. Each particular piece of code
// is owned by the part that added it to the library.
// SSC source code and binaries can only be used by the owners
// and those with explicit permission from the owners.
// SSC shall not be distributed to anyone else.
//
// SSC is distributed WITHOUT ANY WARRANTY; without even
// the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.
//
// See sscLicense.txt for more information.

#ifndef CXDOUBLEWIDGETS_H_
#define CXDOUBLEWIDGETS_H_

#include <QWidget>
#include <QSlider>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QGridLayout>
#include <QDial>

#include "cxDoubleRange.h"
#include "cxDoubleDataAdapter.h"
#include "cxOptimizedUpdateWidget.h"

namespace cx
{
class MousePadWidget;

/**\brief Custom widget for display of double-valued data.
 *
 * Use the double-named methods instead of qslider's int-based ones.
 *
 * \ingroup cx_resource_widgets
 */
class DoubleSlider: public QSlider
{
Q_OBJECT

public:
	virtual ~DoubleSlider()
	{
	}
	DoubleSlider(QWidget* parent = 0) :
		QSlider(parent)
	{
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueChangedSlot(int)));
		setDoubleRange(DoubleRange(0, 1, 0.1));
	}
	void setDoubleRange(const DoubleRange& range)
	{
		mRange = range;
		setRange(0, mRange.resolution());
		setSingleStep(mRange.step());
	}
	void setDoubleValue(double val)
	{
		int v_i = (val - mRange.min()) / mRange.step();
		setValue(v_i);
	}
	double getDoubleValue() const
	{
		double v_d = mRange.step() * (double) value() + mRange.min();
		return v_d;
	}
	void setDoubleTickInterval(double interval)
	{
		setTickInterval(interval / mRange.step());
	}

signals:
	void doubleValueChanged(double value);

private slots:
	void valueChangedSlot(int val)
	{
		emit doubleValueChanged(getDoubleValue());
	}

private:
	DoubleRange mRange;
};

/**\brief A QLineEdit specialized to deal with double data.
 *
 * \ingroup sscWidget
 */
class DoubleLineEdit: public QLineEdit
{
public:
	DoubleLineEdit(QWidget* parent = 0) :
		QLineEdit(parent)
	{
	}
	virtual QSize sizeHint() const;
	virtual QSize minimumSizeHint() const;
	double getDoubleValue(double defVal = 0.0) const
	{
		bool ok;
		double newVal = this->text().toDouble(&ok);
		if (!ok)
			return defVal;
		return newVal;
	}
	void setDoubleValue(double val)
	{
		this->setText(QString::number(val, 'g', 4));
	}
};

/**\brief Composite widget for scalar data manipulation.
 *
 * Consists of <namelabel, valueedit, slider>.
 * Insert a subclass of DoubDoubleDataAdapter order to connect to data.
 *
 * Before use: enable the widgets you need, then add either to Own layout, in case you wish to
 * use the widget normally, or add to grid, in case you wish to integrate the elements into a
 * larger grid.
 *
 * \ingroup cx_resource_widgets
 */
class ScalarInteractionWidget: public OptimizedUpdateWidget
{
Q_OBJECT
public:
	ScalarInteractionWidget(QWidget* parent, DoubleDataAdapterPtr);

	void enableLabel();
	void enableSlider();
	void enableDial();
	void enableEdit();
	void enableSpinBox();
	void enableInfiniteSlider();

	void addToOwnLayout();
	void addToGridLayout(QGridLayout* gridLayout = 0, int row = 0);
	void build(QGridLayout* gridLayout = 0, int row = 0);


private slots:
	void textEditedSlot();
	void doubleValueChanged(double val);
	void intValueChanged(int val); //expecting a value that is 100 times as big as the double value (because QDial uses ints, not double, we scale)
	void infiniteSliderMouseMoved(QPointF delta);

protected:
    virtual void prePaintEvent();
	DoubleDataAdapterPtr mData;

private:
	void enableAll(bool);

	DoubleSlider* mSlider;
	QDial* mDial;
	QDoubleSpinBox* mSpinBox;
	QLabel* mLabel;
	DoubleLineEdit* mEdit;
	MousePadWidget* mInfiniteSlider;
};

/**\brief Composite widget for scalar data manipulation.
 *
 * Consists of <namelabel, valueedit, slider>.
 * Insert a subclass of DoubDoubleDataAdapter order to connect to data.
 *
 * \ingroup cx_resource_widgets
 */
class SliderGroupWidget: public ScalarInteractionWidget
{
Q_OBJECT
public:
	SliderGroupWidget(QWidget* parent, DoubleDataAdapterPtr, QGridLayout* gridLayout = 0, int row = 0);
};

/**\brief Composite widget for scalar data manipulation.
 *
 * Consists of <namelabel, valueedit, slider>.
 * Insert a subclass of DoubDoubleDataAdapter order to connect to data.
 *
 * Uses a QDoubleSpinBox instead of a slider - this gives a more compact widget.
 *
 * \ingroup cx_resource_widgets
 */
class SpinBoxGroupWidget: public ScalarInteractionWidget
{
Q_OBJECT
public:
	SpinBoxGroupWidget(QWidget* parent, DoubleDataAdapterPtr, QGridLayout* gridLayout = 0, int row = 0);
};

/**\brief Composite widget for scalar data manipulation.
 *
 * Consists of <namelabel, valueedit, slider>.
 * Insert a subclass of DoubDoubleDataAdapter in order to connect to data.
 *
 * Uses both a slider and a spin box
 *
 * \ingroup cx_resource_widgets
 */
class SpinBoxAndSliderGroupWidget: public ScalarInteractionWidget
{
Q_OBJECT
public:
	SpinBoxAndSliderGroupWidget(QWidget* parent, DoubleDataAdapterPtr, QGridLayout* gridLayout = 0, int row = 0);
};
typedef boost::shared_ptr<SpinBoxAndSliderGroupWidget> SpinBoxAndSliderGroupWidgetPtr;

/**\brief Composite widget for scalar data manipulation.
 *
 * Consists of <namelabel, valueedit, dial>.
 * Insert a subclass of DoubDoubleDataAdapter in order to connect to data.
 *
 * Uses both a dial and a spin box
 *
 * \ingroup cx_resource_widgets
 */
class SpinBoxAndDialGroupWidget: public ScalarInteractionWidget
{
Q_OBJECT
public:
SpinBoxAndDialGroupWidget(QWidget* parent, DoubleDataAdapterPtr, QGridLayout* gridLayout = 0, int row = 0);
};
typedef boost::shared_ptr<SpinBoxAndDialGroupWidget> SpinBoxAndDialGroupWidgetPtr;


/**\brief Composite widget for scalar data manipulation.
 *
 * Consists of <namelabel, valueedit, slider>.
 * Insert a subclass of DoubDoubleDataAdapter order to connect to data.
 *
 * This slider is custom-made and infinitely long.
 *
 * \ingroup cx_resource_widgets
 */
class SpinBoxInfiniteSliderGroupWidget: public ScalarInteractionWidget
{
Q_OBJECT
public:
	SpinBoxInfiniteSliderGroupWidget(QWidget* parent, DoubleDataAdapterPtr, QGridLayout* gridLayout = 0, int row = 0);
};

} //namespace cx


#endif /* CXDOUBLEWIDGETS_H_ */
