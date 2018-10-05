# shift.tools.rc - Resource Compiler

This console executable is a lightweight frontend for the [shift.rc](../../rc/doc/rc.md) library.

## Commandline Options
```
Allowed options:
  --help                               Shows this help message.
  --silent                             Disables logging to console.
  --no-logfile                         Disables logging to file.
  --log-level arg (=warn)              Selects a log level (may be one of
                                       'debug', 'info', 'warn', or 'error')
  --log-arguments                      Writes all program arguments to the log.
  --show-console arg (=1)              Show or hide the console window
  -i [ --input ] arg (="resources")    Base path to all source files to run
                                       through the resource compiler.
  -b [ --build ] arg (="build-rc")     Base path to write temporary resource
                                       files to.
  -o [ --output ] arg (="production")  Base path to write compiled files to.
  -r [ --rules ] arg (=.rc-rules.json) Name of rules json files to search for.
  -c [ --cache ] arg (=.rc-cache.json) Name of a cache json file used to store
                                       private data which is used to improve
                                       performance of subsequent rc
                                       invocations.
  -v [ --verbose ] [=arg(=1)] (=0)     Print more information.
  --image-magick arg (=magick)         Image Magick's command line executable.
  --task-num-workers arg (=0)          Number of worker threads to use to
                                       process tasks. The default value of zero
                                       lets the application automatically chose
                                       the number of threads.
  --floating-point-exceptions arg (=1) Enable floating-point exceptions.
```
