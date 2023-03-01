THRIFT_SOURCE="../"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --thrift-source)
      THRIFT_SOURCE="$2"
      shift 2
      ;;
    *)
      echo "Unknown option: $1"
      exit 1
      ;;
  esac
done

rm -rf ./generated
rm -rf ./thrift

mkdir -p thrift

for file in $(find "$THRIFT_SOURCE" -type f -name '*.thrift'); do
  filename=$(basename "$file")
  if [ ! -f "./thrift/$filename" ]; then
    cp "$file" "./thrift/"
  fi
done

for file in $(find ./thrift -type f -name '*.thrift'); do
  sed -i 's/include "\(.*\)\/\([^\/]*\)"/include "\2"/g' "$file"
done

npm run thrift-ts
mv codegen generated
rm -rf ./thrift

for file in $(find ./generated -type f -name '*.ts'); do
  sed -i 's/import Int64 = require("node-int64");/import Int64 from "node-int64";/' "$file"
done

