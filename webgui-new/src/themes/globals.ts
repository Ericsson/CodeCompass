import { Inter } from '@next/font/google';

const inter = Inter({ subsets: ['latin'] });

export const globalStyles = `
* {
  box-sizing: border-box;
  padding: 0;
  margin: 0;
}

body {
  height: 100vh;
  font-family: ${inter.style.fontFamily};
}
`;
