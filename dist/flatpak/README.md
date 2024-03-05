# Flatpak

This is a disfunctional prototype.

## State
After a second approach to flatpak the app, still not a good idea to bundle Albert for linux using flatpak.
Some reasons:

* / mappped to /run/host
  * Root browser?
  * App extension, QStandardPaths::ApplicationsLocation, desktop entry spec application dirs?
* Container PATH is not host PATH (/run/host/bin)
  * albert::runDetachedProcess?
  * QProcess?
  * POpen?
  * Terminals?
* Dependency mgmt
  * pip, venv: container, host, mixed?
  * LD_PATH: Optional dependencies?

## Useful stuff
```
flatpak list
flatpak install org.kde.Runtime
flatpak install org.kde.Sdk
flatpak-builder --install --user --force-clean build-dir org.albertlauncher.Albert.yml
flatpak run --command=sh --devel org.albertlauncher.Albert
```

## Conclusion
Sandboxing is fine, but we actually just need standardized distribution of software for linux.

