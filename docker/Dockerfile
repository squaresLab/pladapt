FROM consol/ubuntu-xfce-vnc:1.2.3
LABEL Description="Docker image for DART Simulator with VNC"

USER root

# first install dependencies
RUN apt update && apt install -y --no-install-recommends git libboost-all-dev \
libyaml-cpp-dev make automake autoconf g++ default-jdk ant wget libtool gdb

# now install eclipse
# download eclipse-cpp
RUN wget http://ftp.osuosl.org/pub/eclipse/technology/epp/downloads/release/2018-09/R/eclipse-cpp-2018-09-linux-gtk-x86_64.tar.gz
# extract eclipse to /opt
RUN tar --directory=/opt -xzf eclipse-cpp-2018-09-linux-gtk-x86_64.tar.gz && \
rm eclipse-cpp-2018-09-linux-gtk-x86_64.tar.gz
# add eclipse to PATH
ENV PATH $PATH:/opt/eclipse

# now clone and build DART
WORKDIR $HOME
RUN git clone https://github.com/cps-sei/pladapt.git
WORKDIR pladapt
ENV PLADAPT $(pwd)

# build DART
# if PLA-SDP is desired, generate reachability functions
#RUN reach/build.sh
RUN autoreconf -i && mkdir build; cd build; ../configure && make
# compile the Java wrapper
#WORKDIR $PLADAPT/java
#RUN make

# add projects to eclipse workspace
ADD eclipse-workspace /root/eclipse-workspace

# create desktop shortcut
ADD eclipse.desktop /headless/Desktop

# Cleanup
RUN apt-get clean && \
  rm -rf /var/lib/apt

WORKDIR $HOME
