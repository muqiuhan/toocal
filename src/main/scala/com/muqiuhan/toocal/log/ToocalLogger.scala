package com.muqiuhan.toocal.log

import scribe.Logging
import scribe.format.*
import scribe.Level
import java.time.format.DateTimeFormatter
import java.time.LocalDateTime
import scribe.format.FormatBlock.*

object ToocalLogger extends Logging:
  scribe.Logger.root
    .clearHandlers()
    .clearModifiers()
    .withHandler(
      minimumLevel = Some(Level.Debug),
      formatter =
        formatter"| $date ${string("[")}$levelColored${string("]")} ${green(ClassNameSimple)} - $messages$mdc"
    )
    .replace()

  val log = logger
