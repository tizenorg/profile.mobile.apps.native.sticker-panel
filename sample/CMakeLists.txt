SET(SAMPLE "sample")

INCLUDE(FindPkgConfig)
pkg_check_modules(sample_pkgs REQUIRED
	dlog
	elementary
	appcore-efl
)

FOREACH(sample_flag ${sample_pkgs_CFLAGS})
	SET(SAMPLE_EXTRA_CFLAGS "${SAMPLE_EXTRA_CFLAGS} ${sample_flag}")
ENDFOREACH(sample_flag)

SET(SAMPLE_EXTRA_CFLAGS, "${SAMPLE_EXTRA_CFALGS} -DPREFIX=\"${CMAKE_INSTALL_PREFIX}\"")

ADD_EXECUTABLE(${SAMPLE}
	sample.c
	sticker_icon.c
)

TARGET_LINK_LIBRARIES(${SAMPLE} ${PROJECT_NAME} ${sample_pkgs_LDFLAGS} ${SAMPLE_EXTRA_LDFLAGS})

# install edj
ADD_CUSTOM_TARGET(sample.edj
		COMMAND edje_cc	${CMAKE_CURRENT_SOURCE_DIR}/sample.edc sample.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/sample.edc
)
ADD_DEPENDENCIES(${SAMPLE} sample.edj)

INSTALL(TARGETS ${SAMPLE} DESTINATION ${SAMPLEDIR})
INSTALL(FILES sample.edj DESTINATION ${SAMPLEDIR})

