# Fortify on Demand: C/C++ scanning in GitHub Actions

Reference pipeline for scanning C/C++ code with Fortify on Demand (FoD) from GitHub Actions.

FoD does not accept C/C++ source directly. The code is **translated** in the build environment with
OpenText Fortify SAST (`sourceanalyzer`), exported as a **Mobile Build Session (MBS)**, and the MBS is
submitted to FoD. Results come back into **GitHub code scanning**.

```
push / PR
  -> container job (private image: Fortify SAST + fcli + gcc/make)
  -> sourceanalyzer touchless make      translate using the real build
  -> -export-build-session              package as MBS (zip)
  -> fortify/github-action (USE_PACKAGE) scan in FoD (stack: MBS/C/C++/Scala), wait
  -> github-sast-report export          SARIF with data flow
  -> GitHub code scanning
```

`src/` is a deliberately vulnerable demo app (buffer overflow, format string, path manipulation,
hard-coded password and more). **Do not deploy it.**

## Repository layout

| Path | Purpose |
|---|---|
| `src/`, `Makefile` | Demo C application, built with `make` |
| `docker/Dockerfile` | Image with Fortify SAST, fcli and the C/C++ toolchain |
| `docker/installer/` | Put the SAST Linux installer here before building (git-ignored, never commit it) |
| `.github/workflows/fortify-sast-cpp.yml` | Translate and export MBS, then the Fortify GitHub Action scans in FoD and publishes to code scanning |

## Setup

### 1. Build and push the image (once per SAST version)

Download **SCA Linux** from the FoD **Tools** page. Copy the `.run` installer into `docker/installer/`.

```bash
DOCKER_BUILDKIT=1 docker build \
  --secret id=fortify_license,src=/path/to/fortify.license \
  --build-arg SAST_INSTALLER=SCA_Linux_Latest_x64.run \
  -t ghcr.io/<owner>/fortify-sast-cpp:26.3.0 docker/

docker push ghcr.io/<owner>/fortify-sast-cpp:26.3.0
```

The licence is passed as a build secret and removed after install, so it is not stored in the image.
**Keep the image private**: it contains a licensed product.

### 2. FoD release

Create (or reuse) an application and release, and set **Static Scan Setup > Technology Stack** to
**MBS/C/C++/Scala**.

### 3. Repository secrets and variables

| Type | Name | Value |
|---|---|---|
| Secret | `FORTIFY_LICENSE_B64` | `base64 -w0 fortify.license` |
| Secret | `FOD_CLIENT_ID` | FoD API key ID (permission to start scans) |
| Secret | `FOD_CLIENT_SECRET` | FoD API key secret |
| Variable | `FOD_URL` | Your FoD API URL |
| Variable | `FOD_RELEASE` | `<application>:<release>` |

The workflow skips until `FOD_RELEASE` is set.

## Using this for your own C/C++ projects

- **The build must work in the image.** Add your toolchain and dependencies to the Dockerfile
  (`cmake`, `libssl-dev`, cross-compilers and so on). Touchless translation captures real compiler calls,
  so if it doesn't compile, it isn't translated.
- **Replace `make`** in the Translate step with your build command (`cmake --build build`, `./build.sh`).
- **Check the translated file list.** The workflow fails if nothing was translated; `translate.log` is
  kept as a build artefact.
- **Compiler not recognised?** Wrap it explicitly instead of touchless:
  `make CC="sourceanalyzer -b $BUILD_ID gcc"`.

The scan step uses the standard Fortify GitHub Action (`fortify/github-action@v3`), the same action used for
Java, .NET or IaC repos. The only C/C++ difference is the translate and export steps before it, with
`USE_PACKAGE` pointing the action at the MBS instead of letting it package source.

Reference: Fortify on Demand 26.4 User Guide, section 4.1.2.8 "Prepare C and C++ application files".
