package com.muqiuhan.toocal.core

case class Config(
    pageSize: Int = Page.DATA_SIZE,
    minFillPercent: Float,
    maxFillPercent: Float
)

object Config:
    val DEFAULT: Config = Config(minFillPercent = 0.5, maxFillPercent = 0.95)
