# Frontend development guide

## Prerequisites

Make sure you are using at least version 18 of [Node.js](https://nodejs.org/en/).\
You can also download [nvm](https://github.com/nvm-sh/nvm), and run `nvm install` at the root of the project to use the specified Node version (18.13.0).

## Development

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

You can set the URL of the webserver (`BACKEND_URL`) and the port of the development server (`DEVSERVER_PORT`) by creating a `.env.local` file at the root of the project. This will override the variables in the `.env` file.\
The default value is `http://localhost:8080` for the webserver URL and `3000` for the port of the development server, as seen in the `.env` file.\

If the frontend has a subpath specified (e.g `codecompass.net` has the `/demo` path), you can set this with the `PUBLIC_URL` variable.

```py
# .env.local
PUBLIC_URL=
BACKEND_URL=
DEVSERVER_PORT=
```

After this, you can run the development server with:

```bash
npm run dev
```

Open [http://localhost:3000](http://localhost:3000) with your browser to see the result.\
(Make sure to open the URL with the correct port if you chose to override the port for the devserver.)

## Learn More

This is a [Next.js](https://nextjs.org/) project bootstrapped with [`create-next-app`](https://github.com/vercel/next.js/tree/canary/packages/create-next-app).

To learn more about Next.js, take a look at the following resources:

- [Next.js Documentation](https://nextjs.org/docs) - learn about Next.js features and API.
- [Learn Next.js](https://nextjs.org/learn) - an interactive Next.js tutorial.

You can check out [the Next.js GitHub repository](https://github.com/vercel/next.js/) - your feedback and contributions are welcome!

## Build and Install

The necessary steps for building and installing the application can be found in the [deps](./deps.md) and [usage](./usage.md) guides.\
(Note: It is recommended to use SQLite as a database for parsing projects in development.)

After installing, the current webgui will still be available at the root URL of the webserver (`/`), but you can also reach the new webgui at (`/new`).
