

Each plugin exposes a"C" interface of four functions:

  - plugin_getname() - return the "name" of the plugin
  - plugin_gettype() - return the "type" of the plugin
  - plugin_create_xxx() - return an instance of the plugin device
  - plugin_getdevices_xxx() - return a list of device names

This abstracts a general interface to any plugin. 

Since new plugins may want to use different types of create functions,
or in fact have any kind of functions, using a simple set of static
functions starts to get clumsy. However, it would be easy to provide a
function that allows you obtain specific functions BY NAME. This is
easy - it already done by the PDynaLink class.

This is actually a very well known technique. It is the way that
plugin technologies like COM, OCX and ActiveX operate.
With this technique, every plugin exposes the following functions:

        GetVersion() 
	GetName() 
	GetClass() 
	GetFunction()
 
The GetVersion function allows us to specify an API version, so that
we can change the plugin interface at a later date.

GetName() and GetClass() provide the "name" and "type" of the
plugin. These names are the same as the class names of
the plugin, i.e. name = "PSoundChannelOSS" and "PSoundChannel" as
these are guaranteed to be unique and have advantages when using
static linking.

GetFunction is what allows the plugin interface to be completely
generic.  The plugin manager only uses the four functions above (which
are common to all plugins) so it can be completely generic without
having any knowledge at all of what the plugins do.

The base class for each plugin type (such as PSoundChannel) have
knowledge of what functions the plugin must make available via the
GetFunction routine. Hence, the routines for each driver type (i.e.
PSoundChannel) can do what they need to do, and the plugin manager is
still completely generic. There can even be subclasses of plugins
(such as devices) that share common APIs, and the plugin manager is
still completely generic.

This was the design allows the plugin manager to be completely
generic.  Further the underlying design is sound as it does provide
for complete backwards compatibility.

As an aside, it would be possible to only export one function from a
plugin - CreateFunction. The other functions common to all plugins
(GetVersion, GetName and GetClass) could be obtained via
CreateFunction.  While this is certainly possible, having these four
functions export should provide a fairly unique fingerprint that will
ensure only valid .so files are loaded as plugins.

Initial author, Craig Southeren.
Revisions       Derek Smithies.
