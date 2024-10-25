FROM gcc:14.2.0 as build
WORKDIR /usr/src/myapp
COPY main.c /usr/src/myapp
RUN gcc -o mem-eater main.c

FROM ubuntu:oracular
WORKDIR /app
COPY --from=build /usr/src/myapp/mem-eater mem-eater
USER 65534:65534

ENTRYPOINT ["/app/mem-eater"]
CMD ["1","5","10"]