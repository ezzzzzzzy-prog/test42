if [ -n "${OUTPUT_FILE}" ] && [ "${COVERAGE}" != "yes" ]; then
  echo 0 > "${OUTPUT_FILE}"
fi

exit 0
