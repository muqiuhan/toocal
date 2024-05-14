package com.muqiuhan.toocal.core

import com.muqiuhan.toocal.errors

enum Error extends errors.Error:

    case DataBaseFileNotFound
    case DataAccessLayerCannotSeekPage
    case DataAccessLayerCannotReadPage
    case DataAccessLayerCannotWritePage
    case InvalidPageNumber
