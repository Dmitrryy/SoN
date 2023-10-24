macro(upd_tar_list NEW_TAR TARGETS)
  # Append target name to targets list & propagate updated list to parent scope
  list(APPEND ${TARGETS} ${NEW_TAR})
  set(${TARGETS}
      ${${TARGETS}}
      PARENT_SCOPE)
endmacro()
