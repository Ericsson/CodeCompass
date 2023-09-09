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

for file in $(find "$THRIFT_SOURCE/service/" -type f -name '*.thrift'); do
  filename=$(basename "$file")
  echo "Installing $filename"
  if [ ! -f "./thrift/$filename" ]; then
    cp "$file" "./thrift/"
  fi
done

for file in $(find "$THRIFT_SOURCE/plugins/" -type f -name '*.thrift'); do
  filename=$(basename "$file")
  echo "Installing $filename"
  if [ ! -f "./thrift/$filename" ]; then
    cp "$file" "./thrift/"
  fi
done

# Fix include paths
for file in $(find ./thrift -type f -name '*.thrift'); do
  sed -i 's/include "\(.*\)\/\([^\/]*\)"/include "\2"/g' "$file"
done

npm run thrift-ts
rm -rf ./thrift

for file in $(find ./generated -type f -name '*.ts'); do
  sed -i 's/import Int64 = require("node-int64");/import Int64 from "node-int64";/' "$file"
done

# Resolve conflicting import in SearchService.ts
sed -i 's/import \* as SearchResult from "\.\/SearchResult";/import \* as SearchRes from "\.\/SearchResult";/g' ./generated/SearchService.ts
sed -i 's/SearchResult\.SearchResult/SearchRes.SearchResult/g' ./generated/SearchService.ts

# Add missing inclusion dependency in HelmService for LanguageService
sed -i '/import \* as __ROOT_NAMESPACE__ from "\.\/";/a import \* as LanguageService from "\.\/LanguageService";' ./generated/HelmService.ts
sed -i 's/__ROOT_NAMESPACE__\.LanguageService/LanguageService/g' ./generated/HelmService.ts
