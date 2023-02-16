# New Next.js (React) based webgui for CodeCompass

## Development

For easier development, first run the webserver (CodeCompass_webserver), then run the development server.\
Since the webserver and development server are different origins, you have to use the [CORS extension](https://chrome.google.com/webstore/detail/allow-cors-access-control/lhobafahddgcelffkeicbaginigeejlf?hl=en) to resolve CORS issues.

Alternatively, you could add this change to the `MainRequestHandler::begin_request_handler(struct mg_connection* conn_)` method in `mainrequesthandler.cpp`, then build the webserver again.\
(Note: Do not commit this change, as it is unnecessary in production.)

```c
mg_send_header(conn_, "Access-Control-Allow-Origin", "http://localhost:3000");
```

Learn more about CORS [here](https://www.moesif.com/blog/technical/cors/Authoritative-Guide-to-CORS-Cross-Origin-Resource-Sharing-for-REST-APIs/).

Run the webserver with:

```bash
cd <path-to-webserver-binary> && ./CodeCompass_webserver -w <path-to-workspace-folder> -p <port>
```

Install npm packages with:

```bash
npm install
```

Generate files from Thrift with:

```bash
npm run thrift-codegen
```

You can specify the webserver host and port by creating a .env file at the root of the project (webgui-new/), like this:\
(Note: The default values are localhost and 8080, even without a .env file. Make sure the host and port are the same as the webserver.)

```py
# .env
WEBSERVER_HOST=localhost
WEBSERVER_PORT=8080
```

After, this, you can run the development server with:

```bash
npm run dev
```

Open [http://localhost:3000](http://localhost:3000) with your browser to see the result.

## Learn More

This is a [Next.js](https://nextjs.org/) project bootstrapped with [`create-next-app`](https://github.com/vercel/next.js/tree/canary/packages/create-next-app).

To learn more about Next.js, take a look at the following resources:

- [Next.js Documentation](https://nextjs.org/docs) - learn about Next.js features and API.
- [Learn Next.js](https://nextjs.org/learn) - an interactive Next.js tutorial.

You can check out [the Next.js GitHub repository](https://github.com/vercel/next.js/) - your feedback and contributions are welcome!

## Build and Install

For building and installing the application, or parsing projects, please refer to the [CodeCompass README](/README.md) documentations and development sections.\
(Note: For development, use SQLite as a database for parsing projects.)

After installing, the static HTML exported from Next.js will be available in `install/share/codecompass/webgui-new/out/`
