FROM ubuntu:latest

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install Bruce's C Compiler (bcc), 8086 Assembler/Linker (bin86 provides as86 and ld86), and QEMU
RUN apt-get update && apt-get install -y \
    bcc \
    bin86 \
    qemu-system-x86 \
    make \
    bash \
    && rm -rf /var/lib/apt/lists/*

# Set default working directory inside the container
WORKDIR /work

# Default entry shell
CMD ["/bin/bash"]