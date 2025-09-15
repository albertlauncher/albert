{% if version -%}
## {{ version }} ({{ timestamp | date(format="%Y-%m-%d") }})
{%- else %}
## [unreleased]
{%- endif %}

{% if commits -%}
### Core changes
{% for group, commits in commits | group_by(attribute="group") %}
#### {{ group | striptags | trim | upper_first }}

{% for commit in commits -%}
- {% if commit.scope %}_{{ commit.scope }}_ · {% endif -%}
  {% if commit.breaking %}[**BREAKING**] {% endif -%}
  {{ commit.message | upper_first }}
{% endfor -%}
{% endfor -%}
{% endif %}
{% if extra.plugin_contexts -%}
{% set commits = [] -%}
{% for context in extra.plugin_contexts -%}
{% set_global commits = commits | concat(with=context.commits) -%}
{% endfor -%}
### Plugin changes
{% for group, commits in commits | group_by(attribute="group") %}
#### {{ group | striptags | trim | upper_first }}

{% for commit in commits -%}
- **{{ commit.extra.plugin_name }}** 
  {%- if commit.scope %} · _{{ commit.scope }}_{% endif %}
  {%- if commit.breaking %} · [**BREAKING**]{% endif %}
  {%- if commit.message %} · {{ commit.message | upper_first }}{% endif %}
{% endfor -%}
{% endfor -%}
{% endif -%}
 