import qbs.base 1.0
import "../QtcPlugin.qbs" as QtcPlugin

QtcPlugin {
    name: "ProjectExplorer"

    Depends { name: "Qt"; submodules: ["widgets", "xml", "network", "script", "quick1"] }
    Depends { name: "Core" }
    Depends { name: "Locator" }
    Depends { name: "Find" }
    Depends { name: "TextEditor" }
    Depends { name: "QtcSsh" }

    Depends { name: "cpp" }
    cpp.defines: base.concat(["QTC_CPU=X86Architecture"])
    cpp.includePaths: [
        ".",
        "..",
        "../../libs",
        "../../libs/utils",
        "customwizard",
        "publishing",
        buildDirectory
    ]

    files: [
        "doubletabwidget.ui",
        "processstep.ui",
        "projectexplorer.qrc",
        "removefiledialog.ui",
        "sessiondialog.ui",
        "targetsettingswidget.ui",
        "projectwizardpage.ui",
        "abi.h",
        "abiwidget.cpp",
        "abiwidget.h",
        "abstractprocessstep.cpp",
        "abstractprocessstep.h",
        "allprojectsfilter.cpp",
        "allprojectsfind.cpp",
        "allprojectsfind.h",
        "applicationlauncher.cpp",
        "applicationlauncher.h",
        "applicationrunconfiguration.h",
        "appoutputpane.h",
        "baseprojectwizarddialog.cpp",
        "baseprojectwizarddialog.h",
        "buildconfiguration.h",
        "buildconfigurationmodel.cpp",
        "buildconfigurationmodel.h",
        "buildenvironmentwidget.cpp",
        "buildenvironmentwidget.h",
        "buildmanager.h",
        "buildprogress.h",
        "buildsettingspropertiespage.h",
        "buildstep.cpp",
        "buildsteplist.cpp",
        "buildsteplist.h",
        "buildstepspage.h",
        "cesdkhandler.cpp",
        "cesdkhandler.h",
        "clangparser.h",
        "codestylesettingspropertiespage.h",
        "codestylesettingspropertiespage.ui",
        "compileoutputwindow.cpp",
        "compileoutputwindow.h",
        "copytaskhandler.h",
        "corelistenercheckingforrunningbuild.cpp",
        "corelistenercheckingforrunningbuild.h",
        "currentprojectfilter.cpp",
        "currentprojectfind.cpp",
        "currentprojectfind.h",
        "dependenciespanel.h",
        "deployconfiguration.cpp",
        "deployconfiguration.h",
        "deployconfigurationmodel.cpp",
        "deployconfigurationmodel.h",
        "doubletabwidget.h",
        "editorconfiguration.cpp",
        "editorconfiguration.h",
        "editorsettingspropertiespage.h",
        "editorsettingspropertiespage.ui",
        "environmentitemswidget.cpp",
        "environmentwidget.h",
        "foldernavigationwidget.cpp",
        "foldernavigationwidget.h",
        "gccparser.h",
        "gcctoolchainfactories.h",
        "gnumakeparser.h",
        "headerpath.h",
        "ioutputparser.cpp",
        "ioutputparser.h",
        "iprojectmanager.h",
        "iprojectproperties.h",
        "itaskhandler.h",
        "ldparser.h",
        "linuxiccparser.h",
        "metatypedeclarations.h",
        "miniprojecttargetselector.h",
        "namedwidget.cpp",
        "namedwidget.h",
        "nodesvisitor.cpp",
        "nodesvisitor.h",
        "outputparser_test.h",
        "pluginfilefactory.cpp",
        "pluginfilefactory.h",
        "processparameters.cpp",
        "processparameters.h",
        "processstep.cpp",
        "processstep.h",
        "project.cpp",
        "project.h",
        "projectconfiguration.cpp",
        "projectconfiguration.h",
        "projectexplorer_export.h",
        "projectexplorersettingspage.h",
        "projectfilewizardextension.cpp",
        "projectfilewizardextension.h",
        "projectmodels.cpp",
        "projectmodels.h",
        "projectnodes.cpp",
        "projectnodes.h",
        "projecttreewidget.h",
        "projectwelcomepage.h",
        "projectwindow.cpp",
        "projectwindow.h",
        "projectwizardpage.h",
        "removefiledialog.cpp",
        "removefiledialog.h",
        "runconfigurationmodel.cpp",
        "runconfigurationmodel.h",
        "runsettingspropertiespage.cpp",
        "runsettingspropertiespage.h",
        "session.h",
        "sessiondialog.h",
        "settingsaccessor.h",
        "showineditortaskhandler.cpp",
        "showineditortaskhandler.h",
        "showoutputtaskhandler.cpp",
        "showoutputtaskhandler.h",
        "target.cpp",
        "target.h",
        "targetselector.cpp",
        "targetselector.h",
        "targetsettingspanel.cpp",
        "targetsettingspanel.h",
        "targetsettingswidget.cpp",
        "targetsettingswidget.h",
        "task.cpp",
        "task.h",
        "taskhub.cpp",
        "taskhub.h",
        "taskmodel.h",
        "taskwindow.h",
        "toolchain.cpp",
        "toolchain.h",
        "toolchainconfigwidget.cpp",
        "toolchainconfigwidget.h",
        "toolchainmanager.h",
        "toolchainoptionspage.h",
        "toolchainoptionspage.ui",
        "vcsannotatetaskhandler.h",
        "environmentitemswidget.h",
        "abi.cpp",
        "allprojectsfilter.h",
        "applicationrunconfiguration.cpp",
        "appoutputpane.cpp",
        "buildconfiguration.cpp",
        "buildmanager.cpp",
        "buildprogress.cpp",
        "buildsettingspropertiespage.cpp",
        "buildstep.h",
        "buildstepspage.cpp",
        "clangparser.cpp",
        "codestylesettingspropertiespage.cpp",
        "copytaskhandler.cpp",
        "currentprojectfilter.h",
        "dependenciespanel.cpp",
        "doubletabwidget.cpp",
        "editorsettingspropertiespage.cpp",
        "environmentwidget.cpp",
        "gccparser.cpp",
        "gcctoolchain.cpp",
        "gcctoolchain.h",
        "gnumakeparser.cpp",
        "ldparser.cpp",
        "linuxiccparser.cpp",
        "localapplicationruncontrol.cpp",
        "localapplicationruncontrol.h",
        "miniprojecttargetselector.cpp",
        "outputparser_test.cpp",
        "projectexplorer.cpp",
        "projectexplorer.h",
        "projectexplorerconstants.h",
        "projectexplorersettings.h",
        "projectexplorersettingspage.cpp",
        "projectexplorersettingspage.ui",
        "projecttreewidget.cpp",
        "projectwelcomepage.cpp",
        "projectwizardpage.cpp",
        "runconfiguration.cpp",
        "runconfiguration.h",
        "session.cpp",
        "sessiondialog.cpp",
        "settingsaccessor.cpp",
        "taskmodel.cpp",
        "taskwindow.cpp",
        "toolchainmanager.cpp",
        "toolchainoptionspage.cpp",
        "vcsannotatetaskhandler.cpp",
        "customwizard/customwizard.h",
        "customwizard/customwizardpage.cpp",
        "customwizard/customwizardpage.h",
        "customwizard/customwizardparameters.h",
        "customwizard/customwizardpreprocessor.cpp",
        "customwizard/customwizardpreprocessor.h",
        "customwizard/customwizardscriptgenerator.cpp",
        "customwizard/customwizardscriptgenerator.h",
        "customwizard/customwizard.cpp",
        "customwizard/customwizardparameters.cpp",
        "images/BuildSettings.png",
        "images/CodeStyleSettings.png",
        "images/DeviceConnected.png",
        "images/DeviceDisconnected.png",
        "images/DeviceReadyToUse.png",
        "images/Desktop.png",
        "images/EditorSettings.png",
        "images/MaemoDevice.png",
        "images/ProjectDependencies.png",
        "images/RunSettings.png",
        "images/SymbianDevice.png",
        "images/SymbianEmulator.png",
        "images/build.png",
        "images/build_32.png",
        "images/build_small.png",
        "images/clean.png",
        "images/clean_small.png",
        "images/closetab.png",
        "images/compile_error.png",
        "images/compile_warning.png",
        "images/debugger_start.png",
        "images/debugger_start_small.png",
        "images/findallprojects.png",
        "images/findproject.png",
        "images/leftselection.png",
        "images/midselection.png",
        "images/projectexplorer.png",
        "images/rebuild.png",
        "images/rebuild_small.png",
        "images/rightselection.png",
        "images/run.png",
        "images/run_small.png",
        "images/session.png",
        "images/stop.png",
        "images/stop_small.png",
        "images/targetaddbutton.png",
        "images/targetaddbutton_disabled.png",
        "images/targetbuildselected.png",
        "images/targetpanel_bottom.png",
        "images/targetpanel_gradient.png",
        "images/targetremovebutton.png",
        "images/targetremovebutton_disabled.png",
        "images/targetrunselected.png",
        "images/targetseparatorbackground.png",
        "images/targetunselected.png",
        "images/window.png",
        "publishing/publishingwizardselectiondialog.ui",
        "publishing/ipublishingwizardfactory.h",
        "publishing/publishingwizardselectiondialog.cpp",
        "publishing/publishingwizardselectiondialog.h",
        "devicesupport/idevice.h",
        "devicesupport/idevice.cpp",
        "devicesupport/desktopdevice.cpp",
        "devicesupport/desktopdevice.h",
        "devicesupport/desktopdevicefactory.cpp",
        "devicesupport/desktopdevicefactory.h",
        "devicesupport/devicemanager.h",
        "devicesupport/devicemanager.cpp",
        "devicesupport/devicemanagermodel.h",
        "devicesupport/devicemanagermodel.cpp",
        "devicesupport/devicesettingspage.h",
        "devicesupport/devicesettingspage.cpp",
        "devicesupport/devicefactoryselectiondialog.cpp",
        "devicesupport/devicefactoryselectiondialog.h",
        "devicesupport/devicefactoryselectiondialog.ui",
        "devicesupport/devicesettingswidget.cpp",
        "devicesupport/devicesettingswidget.h",
        "devicesupport/devicesettingswidget.ui",
        "devicesupport/idevicewidget.h",
        "devicesupport/idevicefactory.cpp",
        "devicesupport/idevicefactory.h"
    ]

    Group {
        condition: qbs.targetOS == "windows"
        files: [
           "wincetoolchain.cpp",
           "wincetoolchain.h",
           "windebuginterface.cpp",
           "windebuginterface.h",
           "msvcparser.cpp",
           "msvcparser.h",
           "msvctoolchain.cpp",
           "msvctoolchain.h",
           "abstractmsvctoolchain.cpp",
           "abstractmsvctoolchain.h"
        ]
    }
}

