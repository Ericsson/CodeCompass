namespace cpp cc.service.authentication

exception IncorrectCredentials {}

/**
 * The AuthenticationService is used to manage requesting privileged access to
 * a CodeCompass server. This service MUST always be available without a valid
 * session - as using this service is necessary to obtain a session.
 */
service AuthenticationService
{
  /**
   * Returns whether the server is configured to not allow unprivileged access.
   */
  bool isRequiringAuthentication(),

  /**
   * Returns if the session identified in the current request grants access.
   * If true, the client SHOULD expect to be able to issue further requests.
   * If false, the client MUST obtain a valid session using the Authentication
   * service.
   */
  bool isCurrentSessionValid(),

  /**
   * Returns a free-form text the administrator has set as their server's
   * description.
   */
  string getAuthPrompt(),

  /**
   * Performs authentication on the server using username and password.
   *
   * Returns the session identifier cookie in HTTP header format: the server's
   * accepted cookie name and the cookie value itself joined by a =. Example:
   *   CodeCompassCookie=12345678
   */
  string loginUsernamePassword(1:string username,
                               2:string password)
                              throws (1:IncorrectCredentials credErr),

  /**
   * Destroys the session associated with the request, if it was valid.
   * The session MUST NOT be used afterwards.
   *
   * The call returns the name of the session cookie that was destroyed, e.g.:
   *   CodeCompassCookie
   */
  string logout(),

  /**
   * Returns the user's identifier (usually a user name) that is associated
   * with the current session.
   */
  string getLoggedInUser()
}
