ARG UBUNTU_VER=20.04
FROM ubuntu:$UBUNTU_VER

WORKDIR /home
SHELL ["/bin/bash", "-c"]

ARG APT_PKG_LIST=docker/externals/apt-pkg-list.txt
COPY ${APT_PKG_LIST} /home/apt-pkg-list.txt

RUN apt-get update -y && \
    apt-get upgrade -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        $(cat apt-pkg-list.txt) && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*


ARG SSH_PVT_KEY
COPY $SSH_PVT_KEY /home/id_rsa
COPY ${SSH_PVT_KEY}.pub /home/id_rsa.pub

RUN mkdir /run/sshd && \
    mkdir ~/.ssh && \
    mv id_rsa id_rsa.pub ~/.ssh && \
    echo "StrictHostKeyChecking no" >> /etc/ssh/ssh_config && \
    echo "PermitRootLogin yes" >> /etc/ssh/sshd_config && \
    eval `ssh-agent -s` && \
    ssh-add ~/.ssh/id_rsa && \
    cat ~/.ssh/id_rsa.pub >> ~/.ssh/authorized_keys


ARG PIP_REQ_FILE=requirements.txt
COPY $PIP_REQ_FILE /home/requirements.txt

RUN pip install -r requirements.txt \
        --extra-index-url \
        https://download.pytorch.org/whl/cpu

WORKDIR /home/ray-de
ENTRYPOINT service ssh restart && bash 