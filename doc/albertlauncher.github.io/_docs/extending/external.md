---
layout: docs
title: External plugins
permalink: /docs/extending/external/
---

Albert can be extended using regular executables. They are used like plugins, however the executables are separate processes which have separate address spaces. Therefore these executables are called *external extensions*.

An external plugin basically acts as a listener. The plugin listens on the `stdin` data stream for requests and responds using the `stdout` channel. Generally such requests consist of a mandatory command and optional parameters. The set of commands, the expected responses and their format are defined in the section *Communication protocol*.

The great flexibility of the external plugins comes at a price: Since an external plugin is a separate process it has no access to the core components and helper classes. This holds the other way around as well,which means actions of the results passed to the core application are restricted to the execution of programs with parameters. Further the time to respond to the requests is limited.

#### Communication protocol

`INITIALIZE` is sent when the plugin has been loaded. Upon receiving this message the plugin shall initialize itself and check for dependencies. If no errors occurred in the initialization the plugin has to send `ACK`. If the response does not contain `ACK` the return message will be interpreted as an error string and the plugin will be unloaded. The process has 10 seconds to initialize itself. If the timeout is exceeded the plugin will be killed/unloaded forcefully.

`FINALIZE` is sent when the plugin is about to be unloaded. The plugin is assumed to finalize itself and quit the process. The process has 10 seconds to finalize itself. If the timeout is exceeded the plugin will be killed/unloaded forcefully.

`SETUPSESSION`/`TEARDOWNSESSION` are sent when the user started/ended a session. This is intended to trigger preparation/cleanup of a incoming/past queries. This operation has no timeout and no response is expected. However be aware that since this operation is not synchronized another message, which has a timeout, could have been sent while processing this message.

`QUERY` is the request to handle a query. The rest of the line after the command is the query as the user typed it, i.e. it contains the complete query including spaces and potential triggers. The results of the query have to be returned as a JSON array containing JSON objects representing the results. The plugin has 10 ms to respond to the query. If the timeout is exceeded the plugin will be killed/unloaded forcefully.

A result object has to contain the following values: `id`, `name`, `description`, `icon` and `actions`.

  - `id` is the plugin wide unique id of the result
  - `name` is the name of the result
  - `description` is the description of the result
  - `icon` is the icon of the result
  - `actions` is a JSON array of JSON objects representing the actions for the item.

A JSON object representing an action has to contain the following values: `name`, `command` and `arguments`.

- `name` is the actions name
- `command` is the program to be execute
- `arguments` is an array of parameters for `command`

An example query response could look like this:


```json
[{
  "id":"plugin.wide.unique.id",
  "name":"An Item",
  "description":"Nice description.",
  "icon":"/path/to/icon",
  "actions":[{
    "name":"Action name",
    "command":"program",
    "arguments":["-a"]
  },{
    "name":"Another action name",
    "command":"another_program",
    "arguments":["--name", "value"]
  }]
}]
```
