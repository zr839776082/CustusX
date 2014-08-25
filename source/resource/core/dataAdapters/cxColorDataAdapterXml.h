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
#ifndef CXCOLORDATAADAPTERXML_H_
#define CXCOLORDATAADAPTERXML_H_

#include "cxColorDataAdapter.h"
#include "cxXmlOptionItem.h"


namespace cx
{
typedef boost::shared_ptr<class ColorDataAdapterXml> ColorDataAdapterXmlPtr;

/** DataAdapter for QColor values.
 *
 *
 * \ingroup cx_resource_core_dataadapters
 * \date Nov 22, 2012
 * \author christiana
 */
class ColorDataAdapterXml: public ColorDataAdapter
{
Q_OBJECT
public:
    virtual ~ColorDataAdapterXml()
    {
    }

    /** Make sure one given option exists witin root.
     * If not present, fill inn the input defaults.
     */
    static ColorDataAdapterXmlPtr initialize(const QString& uid, QString name, QString help, QColor value, QDomNode root =
        QDomNode());

public:
    // basic methods
    virtual QString getDisplayName() const; ///< name of data entity. Used for display to user.
	virtual QString getValueAsString() const;
	virtual void setValueFromString(QString value);
    virtual QString getUid() const;
    virtual bool setValue(QColor value); ///< set the data value.
    virtual QColor getValue() const; ///< get the data value.

public:
    // optional methods
    virtual QString getHelp() const; ///< return a descriptive help string for the data, used for example as a tool tip.

signals:
    void valueWasSet(); /// emitted when the value is set using setValue() (similar to changed(), but more constrained)

private:
    QString mName;
    QString mUid;
    QString mHelp;
    QColor mValue;
    XmlOptionItem mStore;

};

} // namespace cx


#endif // CXCOLORDATAADAPTERXML_H_
