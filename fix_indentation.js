#!/usr/bin/env node

// Fix indentation in LPC .c files to use 2-space indentation.
// Converts tabs and 4-space indentation to 2-space indentation.

const fs = require('fs/promises');
const path = require('path');

function convertIndentation(content) {
  return content
    .split(/\r?\n/)
    .map((line) => {
      const expanded = line.replace(/\t/g, '    ');
      const match = expanded.match(/^ +/);
      const leading = match ? match[0].length : 0;

      if (!leading) {
        return expanded;
      }

      const indentLevel = Math.floor(leading / 4);
      const remainder = leading % 4;
      const newIndent = '  '.repeat(indentLevel) + ' '.repeat(remainder);

      return newIndent + expanded.slice(leading);
    })
    .join('\n');
}

async function processFile(filePath) {
  try {
    const originalContent = await fs.readFile(filePath, 'utf8');
    const convertedContent = convertIndentation(originalContent);

    if (originalContent !== convertedContent) {
      await fs.writeFile(filePath, convertedContent, 'utf8');
      console.log(`Fixed: ${filePath}`);
      return true;
    }

    console.log(`No change needed: ${filePath}`);
    return false;
  } catch (err) {
    console.error(`Error processing ${filePath}: ${err.message}`);
    return false;
  }
}

async function collectCFiles(dirPath) {
  const entries = await fs.readdir(dirPath, { withFileTypes: true });
  const files = [];

  for (const entry of entries) {
    const fullPath = path.join(dirPath, entry.name);

    if (entry.isDirectory()) {
      files.push(...(await collectCFiles(fullPath)));
    } else if (entry.isFile() && fullPath.endsWith('.c')) {
      files.push(fullPath);
    }
  }

  return files;
}

async function main() {
  const baseDir = path.resolve(process.argv[2] || process.cwd());

  try {
    const stat = await fs.stat(baseDir);
    if (!stat.isDirectory()) {
      console.error(`Base path is not a directory: ${baseDir}`);
      process.exit(1);
    }
  } catch (err) {
    console.error(`Directory not found: ${baseDir}`);
    process.exit(1);
  }

  const cFiles = await collectCFiles(baseDir);

  if (!cFiles.length) {
    console.error('No .c files found');
    process.exit(1);
  }

  console.log(`Found ${cFiles.length} .c files to process in ${baseDir}`);

  let changed = 0;
  for (const file of cFiles) {
    if (await processFile(file)) {
      changed++;
    }
  }

  console.log('\nProcessing complete!');
  console.log(`Files processed: ${cFiles.length}`);
  console.log(`Files changed: ${changed}`);
  console.log(`Files unchanged: ${cFiles.length - changed}`);
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
