#!/bin/bash

for i in $(which -a convert 2>/dev/null); do
  if [[ -x $i ]]; then
    export IMAGE_MAGICK=$i
    break;
  fi
done

if [[ ! -x $IMAGE_MAGICK ]]; then
  echo "Cannot locate ImageMagick."
  exit 1
fi

export SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
pushd $PWD > /dev/null
cd $SCRIPT_DIR/../

./bin/shift.tools.rc.2_0.x86_64.gcc* \
  --image-magick "${IMAGE_MAGICK}" \
  $@

popd > /dev/null
