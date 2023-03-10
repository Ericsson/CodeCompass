# New Next.js (React) based webgui for CodeCompass

## Prerequisites

Make sure you are using at least version 18 of [Node.js](https://nodejs.org/en/).\
You can also download [nvm](https://github.com/nvm-sh/nvm), and run `nvm install` at the root of the project to use the specified Node version (18.13.0).

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

Generate TypeScript files from Thrift by running the following command:

```bash
chmod +x thrift-codegen.sh && ./thrift-codegen.sh --thrift-source ../
```

You can specify the properties of the webserver by creating a `.env` file at the root of the project. This is only required for development.\
**Important: you have to set the `DEV_ENV` variable to true to be able to use .env variables!**\
The default values are: `localhost` for host, `8080` for port, `false` for https, and path will be an empty string.\
Note: you need to set the `WEBSERVER_PORT`, if you choose to use another port for the webserver, the other properties are not that important considering you run a local webserver.\

```py
# .env
DEV_ENV=true
WEBSERVER_HOST=
WEBSERVER_PORT=
WEBSERVER_HTTPS=
WEBSERVER_PATH=
```

After this, you can run the development server with:

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

After installing, the current webgui will still be available at the root URL of the webserver (`/`), but you can also reach the new webgui at (`/new`).
