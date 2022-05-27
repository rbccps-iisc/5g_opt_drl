DOCKER_NETWORK_NAME=${1:-docker_ray-de}

echo -e \
  $( \
    docker network inspect \
    --format="{{range .Containers}}{{.Name}} : {{.IPv4Address}}\n{{end}}" \
    $DOCKER_NETWORK_NAME \
  ) | sort