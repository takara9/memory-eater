FROM gcc:14.2.0 as build
WORKDIR /usr/src/myapp
COPY main.c /usr/src/myapp
RUN gcc -o mem-eater main.c
RUN touch data && chmod a-r data

FROM alpine:3.20.3
RUN apk add libc6-compat
WORKDIR /
COPY --from=build --chown=65534:65534 /usr/src/myapp/mem-eater /bin/mem-eater
COPY --from=build --chown=65534:65534 /usr/src/myapp/data /data
USER 65534:65534

ENTRYPOINT ["/bin/mem-eater"]
CMD ["1","5","10"]