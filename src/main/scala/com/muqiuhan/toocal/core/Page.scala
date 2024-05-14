package com.muqiuhan.toocal.core

import java.nio.ByteBuffer

type PageNumber = Long

case class Page(var number: PageNumber = -1, data: ByteBuffer)
