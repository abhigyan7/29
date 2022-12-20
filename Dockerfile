FROM gcr.io/distroless/base
COPY bot bot
CMD ["/bot"]
EXPOSE 8001/tcp
