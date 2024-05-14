package com.muqiuhan.toocal.errors

trait Error:

    override def toString: String = s"$this: "

    inline def throwWithMessage(message: String): Nothing =
        throw new Exception(s"${this.toString}: message")

    inline def throwWithException(e: Throwable): Nothing = throwWithMessage(s"${e.getMessage}")
