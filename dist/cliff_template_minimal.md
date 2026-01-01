{% if extra.plugin_contexts -%}
{% set_global commits = commits -%}
{% for context in extra.plugin_contexts -%}
{% set_global commits = commits | concat(with=context.commits) -%}
{% endfor -%}

{% if version -%}
## {{ version }} ({{ timestamp | date(format="%Y-%m-%d") }})
{%- else %}
## [unreleased]
{%- endif %}
{% for group, commits in commits | group_by(attribute="group") %}
### {{ group | striptags | trim | upper_first }}

{% set_global core_commits = [] -%}
{% for commit in commits -%}
{% if not commit.extra.plugin_name -%}
{% set_global core_commits = core_commits | concat(with=commit) -%}
{% endif -%}
{% endfor -%}

{% set_global plugin_commits = [] -%}
{% for commit in commits -%}
{% if commit.extra.plugin_name -%}
{% set_global plugin_commits = plugin_commits | concat(with=commit) -%}
{% endif -%}
{% endfor -%}

{% if core_commits -%}
#### Core

{% for commit in core_commits -%}
- {% if commit.breaking %}[**BREAKING**] {% endif -%}
  {% if commit.scope %}_{{ commit.scope }}_ · {% endif -%}
  {{ commit.message | upper_first }}
{% endfor -%}
{% endif -%}

{% if plugin_commits %}
#### Plugins

{% for name, commits in plugin_commits | group_by(attribute="extra.plugin_name") -%}
{% if commits | length > 1 -%}
- **{{ name }}** 
{%- for commit in commits %}
  - {% if commit.breaking %}[**BREAKING**] · {% endif %}
    {%- if commit.scope %}_{{ commit.scope }}_ · {% endif %}
    {%- if commit.message %}{{ commit.message | upper_first }}{% endif -%}
{% endfor %}
{% else -%}
{% for commit in commits -%}
- **{{ name }}** 
  {%- if commit.breaking %} · [**BREAKING**]{% endif %}
  {%- if commit.scope %} · _{{ commit.scope }}_{% endif %}
  {%- if commit.message %} · {{ commit.message | upper_first }}{% endif %}
{% endfor -%}
{% endif -%}
{% endfor -%}
{% endif -%}

{% endfor -%}
{% endif -%}