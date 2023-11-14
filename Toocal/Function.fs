module Toocal.Core.Function

open ZeroLog

module Retry =
  let logger = LogManager.GetLogger("Retry")

  let (|=) (f) (maxRetries : int) =
    let rec loop retriesRemaining =
      try
        f ()
      with e when retriesRemaining > 0 ->
        logger.Error(e.ToString())
        loop (retriesRemaining - 1)

    loop maxRetries
