# syntax=docker/dockerfile:1.7
#
# OpenText Fortify SAST (sourceanalyzer) + fcli + C/C++ build tools.
# Used by the GitHub workflow to translate C/C++ code into a Mobile Build Session (MBS)
# and submit it to Fortify on Demand.
#
# PRIVATE IMAGE ONLY: it contains the licensed SAST installation. Push to a private registry.
# The licence is NOT baked in: it is mounted as a build secret for the install step only,
# and supplied again at runtime from a CI secret.

FROM ubuntu:24.04

ARG SAST_INSTALLER=OpenText_SAST_Fortify_26.3.0_linux_x64.run
ARG FCLI_URL=https://github.com/fortify/fcli/releases/latest/download/fcli-linux.tgz

ENV DEBIAN_FRONTEND=noninteractive \
    FORTIFY_HOME=/opt/fortify

LABEL org.opencontainers.image.source="https://github.com/vincent-fortify/vdeng-fortify-cpp-demo" \
      org.opencontainers.image.description="Fortify SAST 26.3 + fcli + gcc/make for C/C++ MBS translation"

# Build toolchain for the code being translated. Add project build dependencies here
# (cmake, libssl-dev, etc.): touchless translation only works if the project builds.
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential make zip unzip curl ca-certificates git \
    && rm -rf /var/lib/apt/lists/*

# Silent SAST install. The licence is mounted as a BuildKit secret, used by the installer
# (and the security content update), then deleted in the same layer so it never lands in the image.
COPY installer/${SAST_INSTALLER} /tmp/sast.run
RUN --mount=type=secret,id=fortify_license,target=/tmp/fortify.license \
    chmod +x /tmp/sast.run \
    && /tmp/sast.run --mode unattended \
         --installdir "${FORTIFY_HOME}" \
         --fortify_license_path /tmp/fortify.license \
         --MigrateSCA 0 \
         --UpdateSecurityContent 1 \
    && rm -f /tmp/sast.run "${FORTIFY_HOME}/fortify.license"

# fcli for FoD session, scan submission, wait and SARIF export
RUN curl -fsSL "${FCLI_URL}" | tar -xz -C /usr/local/bin \
    && fcli --version

ENV PATH="${FORTIFY_HOME}/bin:${PATH}"

RUN test -x "${FORTIFY_HOME}/bin/sourceanalyzer"

WORKDIR /src
