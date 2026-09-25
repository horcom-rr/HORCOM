// SPDX-License-Identifier: GPL-3.0-or-later
// horcom, the C++ rewrite of HORCOM by Robert Rettig (1989 to 2010)
// Copyright (c) 2026 Dominik Schwimmbeck

#pragma once

#include <filesystem>
#include <optional>

#include "horcom/data/place_file.hpp"

class QWidget;

// The ORTS-DATEIEN work of his entry box. The hub behind ORTS-DATEIEN :
// HOLEN - EINTRAGEN - LÖSCHEN, the EINTRAGEN path behind ORT
// ABSPEICHERN ? and the VORZUGSORT file ORT.EXT.
namespace horcom {

/// The place file hub of a2ort and a2fort. A FILESELECT over the place
/// files, then DATEI : name with HOLEN, LÖSCHEN and TRIMMEN.
///
/// @param parent   the owning widget
/// @param data_dir the data folder, the place files live in its places
///                 folder beside landnima.int
/// @return the place HOLEN took, nothing for the other branches
[[nodiscard]] std::optional<PlaceRecord> place_file_hub(QWidget* parent, const std::filesystem::path& data_dir);

/// The EINTRAGEN path of a22ort, one place appended to a chosen file,
/// a new name starts a new file.
///
/// @param parent   the owning widget
/// @param data_dir the data folder
/// @param place    the place to enter
/// @return true when the file took the place
bool enter_place(QWidget* parent, const std::filesystem::path& data_dir, const PlaceRecord& place);

/// Stores a place as VORZUGSORT like ortp. An existing one gives way
/// only after his BISHERIGEN VORZUGSORT LÖSCHEN question.
///
/// @param parent   the owning widget
/// @param data_dir the data folder holding ort.ext
/// @param place    the new preferred place
/// @return true when ORT.EXT now holds the place
bool store_preferred_place(QWidget* parent, const std::filesystem::path& data_dir, const PlaceRecord& place);

/// VORZUGSORT LÖSCHEN of his ORT menu, asked like his MESSAGE box.
///
/// @param parent   the owning widget
/// @param data_dir the data folder holding ort.ext
void delete_preferred_place(QWidget* parent, const std::filesystem::path& data_dir);

}  // namespace horcom
