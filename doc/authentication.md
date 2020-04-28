Requiring authentication
========================

In some cases, it might be needed for security or legal reasons to secure the
projects' code available through CodeCompass against unauthorised access.

To enable this feature, an `authentication.json` file should be created under
the workspace directory (`--workspace` or `-w` flag given to the server).

At a bare minimum, to restrict access, an `"enabled": true` MUST be present
in the JSON.

An example valid JSON looks like this:

~~~~{.json}
{
    "enabled": true,
    "prompt": "CodeCompass tutorial server",
    "session_lifetime": 86400,
    "plain": {
        "enabled": true,
        "users": [
            "root:root"
        ]
    }
}
~~~~

 * **`enabled`**: Should be set to `true` if authentication is to be enabled
   on the server. If set to `false`, or the key is missing, authentication will
   be disabled.
 * `prompt`: A text describing the purpose of the server. For the Web client,
   it is shown to users on the log in page.
 * `session_lifetime`: (in seconds) The time it takes from the *last request*
   within that session for it to be permanently invalidated, requiring the
   user to log in again.

Following these keys, the individual *authentication backends* is configured.
The key for each backend is the unique identifier of the backend engine, and
the value is another JSON hash (dictionary), which contains the configuration
specific to that backend.

A common element of all configurations is yet another `enabled` key for each,
allowing toggling of individual backends as necessary.

In the following, the standard authentication engines provided by CodeCompass
are presented. Individual installs might allow for less or more authentication
engines.

Plain username-password from configuration file (`plain`)
---------------------------------------------------------

The allowed users' names and passwords should be listed in the `users`
list. Each entry corresponds to one user. The username and the password should
be separated by `:`.

~~~~{.json}
    "plain": {
        "enabled": true,
        "users": [
            "root:root",
            "guest:guest"
        ]
    }
~~~~


`AuthenticationService` API
---------------------------

Clients wishing to use a CodeCompass server that potentially requires
authentication to access must implement and use the
[`AuthenticationService`](/service/authentication/authentication.thrift) API.
