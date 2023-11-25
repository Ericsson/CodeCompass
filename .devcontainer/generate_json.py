#!/usr/bin/env python3

from json import dumps
from pathlib import Path
from sys import stderr


def main():
    current_folder = Path(__file__).parent
    codecompass_outer_folder = current_folder.parents[1]
    if codecompass_outer_folder.name != "CodeCompass":
        print("Incorrect folder structure: "
              "The cloned repository should be in a directory named 'CodeCompass'!", file=stderr)
        exit(-1)
    json_content = {
        "name": "CodeCompass-devcontainer",
        "image": "modelcpp/codecompass:dev",
        "mounts": [
            {
                "source": str(codecompass_outer_folder),
                "target": "/CodeCompass",
                "type": "bind"
            }
        ],
        "forwardPorts": ["8001:8080"]
    }

    with open(current_folder / "devcontainer.json", "w", encoding="UTF-8") as f:
        f.write(dumps(json_content, indent=2))


if __name__ == "__main__":
    main()
