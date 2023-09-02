from abc import ABC, abstractmethod
from pathlib import PurePath
from typing import Optional


class ImportFinder(ABC):
    # Optional[FileInfo] - circular import
    @abstractmethod
    def get_file_by_location(self, location: PurePath) -> Optional:
        pass

    # Optional[GlobalScope] - circular import
    def get_global_scope_by_location(self, location: PurePath) -> Optional:
        file = self.get_file_by_location(location)
        if file is not None and file.symbol_collector is not None:
            assert file.symbol_collector.scope_manager.get_size() == 1
            return file.symbol_collector.scope_manager.get_global_scope()
        elif file is not None:
            return file.preprocessed_file
        return None

    def get_scope_manager_by_location(self, location: PurePath) -> Optional:
        file = self.get_file_by_location(location)
        if file is not None and file.symbol_collector is not None:
            assert file.symbol_collector.scope_manager.get_size() == 1
            return file.symbol_collector.scope_manager
        return None
