///////////////////////////////////////////////////////////////////////////
// Author: Guillaume Lavou?
// Year: 2011
// CNRS-Lyon, LIRIS UMR 5205
/////////////////////////////////////////////////////////////////////////// 
#ifndef HEADER_MEPP_COMPONENT_MSDM2_PLUGIN_SETTINGS_H
#define HEADER_MEPP_COMPONENT_MSDM2_PLUGIN_SETTINGS_H

#include <mepp_config.h>
#ifdef BUILD_component_MSDM2

#include <QtGui/QDialog>

#include "ui_dialSettings.h"

class SettingsDialog : public QDialog, public Ui_Settings
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent = 0);
    void accept();

private slots:
    void loadDefaults();
    void loadFromSettings();
    void saveToSettings();

private:
};

#endif

#endif // HEADER_MEPP_COMPONENT_MSDM2_PLUGIN_SETTINGS_H