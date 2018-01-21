FROM codecompass

ARG database_host
ARG database_port
ARG database_user_name
ARG database_users_password
ARG webserver_port
ARG shared_workspace_dir

ENV DATABASE_HOST          ${database_host:-localhost}
ENV DATABASE_PORT          ${database_port:-5432}
ENV DATABASE_USER_NAME     ${database_user_name:-postgres}
ENV DATABASE_USER_PASSWORD ${database_users_password:-postgres}
ENV WEBSERVER_PORT         ${webserver_port:-6251}
ENV SHARED_WORKSPACE_DIR   ${shared_workspace_dir:-/data}

# TODO Prepare for configurable user.
WORKDIR /root

COPY start_webserver.sh .
RUN chmod u+x ./start_webserver.sh

EXPOSE ${WEBSERVER_PORT}

CMD ./start_webserver.sh \
    ${DATABASE_HOST} \
    ${DATABASE_PORT} \
    ${DATABASE_USER_NAME} \
    ${DATABASE_USER_PASSWORD} \
    ${WEBSERVER_PORT} \
    ${SHARED_WORKSPACE_DIR}
