module Toocal.Core.Logging

open ZeroLog
open System

type Logging () =
  static let config = (new Configuration.ZeroLogConfiguration())

  do
    config.RootLogger.Appenders.Add(
      new Configuration.AppenderConfiguration(new Appenders.ConsoleAppender())
    )

  static let log = LogManager.Initialize(config)

  interface IDisposable with
    override this.Dispose () = log.Dispose()
