package com.muqiuhan.toocal.core

case class Config(
    val pageSize: Int = Page.DATA_SIZE,
    val minFillPercent: Float,
    val maxFillPercent: Float
)

object Config:
    val DEFAULT = Config(minFillPercent = 0.5, maxFillPercent = 0.95)
