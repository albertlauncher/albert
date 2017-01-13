---
layout: docs
title: Extending Albert
permalink: /docs/extending/
---


Albert has a flexible extension system, which provides users and developers with the ability to extend the functionality by creating *plugins*. These plugins can be used to provide several kinds of functionality by implementing particular interfaces. An instance of such an interface is called an *extension*. A plugin can provide several extensions.

Albert is written in C++ using the Qt framework. Since C++ knowledge is not that widespread and to make extension developers lifes easier, Albert can be extended in several ways.

The native and primary way is to use C++/Qt to write a QPlugin. This gives you the performance of C++, the full set of interfaces to implement, access to several utility classes and direct access to the application and Qt framework including the event loop, which allows asynchronous operations. Read more in the article about [native plugins](/docs/extending/native/).

A simple and extremely flexible way to extend Albert is to use executables. This makes writing plugins less complicated and accessible to a broader community, since *any* language that can be used to build executable files can be used. However this approach is expensive in resource consumption and has several restrictions. Read more in the article about [external plugins](/docs/extending/external/).



However this extensions are separate executables which has several
// implications a developer should be aware of.
//
// * Executable extensions are run and are intended to terminate. This means all
//   state has to be de-/serialized from/to disk. Data intensive extension
//   should therefore implemented natively.
// * An extension is a separate process and therefore has no access to the memory
//   of the core application. This means there is no access to core components,
//   e.g. the app interface itself, icon provider libraries, etc.
// * Although strictly a follow up of the latter it has to be stated that the
//   communication is synchronous in time and fixed by the communication
//   protocol. Its also worth to mention that, due to the synchronicity of the
//   protcol it is not possible to return asynchronous "live" results.
