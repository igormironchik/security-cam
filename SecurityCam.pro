
defineTest(minQtVersion) {
    maj = $$1
    min = $$2
    patch = $$3
    isEqual(QT_MAJOR_VERSION, $$maj) {
        isEqual(QT_MINOR_VERSION, $$min) {
            isEqual(QT_PATCH_VERSION, $$patch) {
                return(true)
            }
            greaterThan(QT_PATCH_VERSION, $$patch) {
                return(true)
            }
        }
        greaterThan(QT_MINOR_VERSION, $$min) {
            return(true)
        }
    }
    greaterThan(QT_MAJOR_VERSION, $$maj) {
        return(true)
    }
    return(false)
}

!minQtVersion(5, 15, 0) {
    message("Cannot build SecurityCam with Qt version $${QT_VERSION}.")
    error("Use at least Qt 5.15.0.")
}

TEMPLATE = subdirs

SUBDIRS = SecurityCam \
	3rdparty
	
SecurityCam.depends = 3rdparty

OTHER_FILES = LICENSE README.md

