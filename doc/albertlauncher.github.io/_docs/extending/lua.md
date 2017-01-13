---
layout: docs
title: Lua modules
permalink: /docs/extending/lua/
---

> Coming soon…




```json
[{
  "id":"extension.wide.unique.id",
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


```c
std::string bla() {
  return "fotze";
}
```


```html
<ul>
  {% raw %}{% for post in site.posts %}{% endraw %}
    <li>
      <a href="{% raw %}{{ post.url }}{% endraw %}">{% raw %}{{ post.title }}{% endraw %}</a>
    </li>
  {% raw %}{% endfor %}{% endraw %}
</ul>
```
