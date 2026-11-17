# Apply this update to your existing Curly project

1. Download curly-release.zip to Downloads. Keep it as a ZIP for the command below.
2. Open Terminal:

```sh
cd ~/Projects/json-parser
git status
unzip -o ~/Downloads/curly-release.zip -d .
make -B test
make example
```

The archive updates the listed source, test, build, documentation, example, and workflow files. It contains no `.git` directory or compiled binaries. Your existing Git history and unrelated files are not removed. `-o` replaces files with matching paths. Review any new local edits before applying; the supplied snapshot showed no tracked modifications.

`make -B test` forces a fresh build with your Mac compiler. Expected totals: 84 parser checks, 16 value checks, and 5 CLI checks. The example prints the name, age, first skill, and a decoded Unicode greeting.

3. Review and stage only the release files:

```sh
git diff --stat
git add include/lexer.h include/parser.h include/json_string.h include/json_value.h src/main.cpp Makefile README.md UPDATE_GUIDE.md tests/test_parser.cpp tests/test_json_value.cpp tests/test_cli.sh tests/run_tests.sh examples/read_profile.cpp examples/profile.json .github/workflows/ci.yml
git diff --cached --stat
git commit -m "Complete JSON value parsing with Unicode decoding tests and CI"
git push
```

Do not use `git add .`: your snapshot included `include/lexer.h.save`, an editor save file, and builds produce additional local outputs. The command above selects the intended source files explicitly.

4. Check the repository's Actions tab for the first workflow run. A push can require workflow-write permission for the GitHub credential. If GitHub rejects it, retain the local commit and follow the specific credential error; do not force-push.

## Understand the change

Previously `parseValue()` returned nothing. It now returns a `JsonValue`. Objects insert each parsed value under its decoded key; arrays append each parsed value. The root value is returned by `parse()`.

Tokens now carry a length as well as a starting byte. This gives the parser the exact original text for each number and quoted string. `decodeString()` turns a quoted token into a real C++ string, including Unicode validation and surrogate-pair decoding.

The command-line validator still prints `Valid JSON` for compatibility. `make example` demonstrates accessing the actual returned values.
