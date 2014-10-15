rall::
	@case '${MFLAGS}' in *[ik]*) set +e;; esac; \
	set -e; for i in $(SUBDIRS) ;\
	do \
		(echo "all" "in *** `pwd`/$$i *** \c"; \
		date;\
		mkdir -p $$i/$(BUILD_DIR);\
		cd $$i && $(MAKE) $(MFLAGS)  all) || exit 1; \
	done

rclean::
	@case '${MFLAGS}' in *[ik]*) set +e;; esac; \
	set -e; for i in $(SUBDIRS) ;\
	do \
		(echo "clean" "in *** `pwd`/$$i *** \c"; \
		date;\
		rm -f $(BUILD_DIR)/*.gcno;\
		cd $$i && $(MAKE) $(MFLAGS)  clean) || exit 1; \
	done

rtest::
	@case '${MFLAGS}' in *[ik]*) set +e;; esac; \
	set -e; for i in $(SUBDIRS) ;\
	do \
		(echo "test" "in *** `pwd`/$$i *** \c"; \
		date;\
		cd $$i && $(MAKE) $(MFLAGS)  test) || exit 1; \
	done

rinstall::
	@case '${MFLAGS}' in *[ik]*) set +e;; esac; \
	set -e; for i in $(SUBDIRS) ;\
	do \
		(echo "install" "in *** `pwd`/$$i *** \c"; \
		date;\
		cd $$i && $(MAKE) $(MFLAGS)  install) || exit 1; \
	done


