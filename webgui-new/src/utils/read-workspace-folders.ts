import fs from 'fs';
import util from 'util';
import { config } from '../service/config';

const workspaceFolder = config.workspace_folder;

const readdir = util.promisify(fs.readdir);
const stat = util.promisify(fs.stat);

export async function readWorkspaceFolders() {
  try {
    const files = await readdir(workspaceFolder);
    let folders = [];

    for (const file of files) {
      const fileStats = await stat(`${workspaceFolder}/${file}`);

      if (fileStats.isDirectory()) {
        folders.push(file);
      }
    }
    return folders;
  } catch (err) {
    console.error(err);
  }
}
