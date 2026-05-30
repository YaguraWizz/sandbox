#include "bimap/bimap.hpp"
#include <deque>
#include <unordered_map>
#include <string>

struct BiMap::Impl {
	Impl() = default;
	Impl(const Impl&) = default;
	Impl(Impl&&) = default;
	Impl& operator=(const Impl&) = default;
	Impl& operator=(Impl&&) = default;

	bool Add(std::string_view key, std::string_view value) {
		// Проверяем, есть ли уже ключ или значение
		if (key_to_value.count(key) > 0 || value_to_key.count(value) > 0) {
			return false;
		}

		// Добавляем данные в deque для долговременного хранения
		deque_pair_key_or_value.emplace_back(std::string(key), std::string(value));
		auto& pair_ref = deque_pair_key_or_value.back();

		// Обновляем отображения
		key_to_value[pair_ref.first] = pair_ref.second;
		value_to_key[pair_ref.second] = pair_ref.first;

		return true;
	}

	std::optional<std::string_view> FindValue(std::string_view key) const noexcept {
		auto it = key_to_value.find(key);
		if (it != key_to_value.end()) {
			return it->second;
		}
		return std::nullopt;
	}

	std::optional<std::string_view> FindKey(std::string_view value) const noexcept {
		auto it = value_to_key.find(value);
		if (it != value_to_key.end()) {
			return it->second;
		}
		return std::nullopt;
	}

	std::deque<std::pair<std::string, std::string>> deque_pair_key_or_value;
	std::unordered_map<std::string_view, std::string_view> key_to_value;
	std::unordered_map<std::string_view, std::string_view> value_to_key;
};


BiMap::BiMap() : impl_ptr(std::make_unique<Impl>()) {}

BiMap::BiMap(const BiMap& other) : impl_ptr(std::make_unique<Impl>(*other.impl_ptr)) {}

BiMap::BiMap(BiMap&& other) noexcept = default;

BiMap& BiMap::operator=(const BiMap& other) {
	if (this != &other) {
		impl_ptr = std::make_unique<Impl>(*other.impl_ptr);
	}
	return *this;
}

BiMap& BiMap::operator=(BiMap&& other) noexcept = default;

BiMap::~BiMap() = default;

bool BiMap::Add(std::string_view key, std::string_view value) {
	return impl_ptr->Add(key, value);
}

std::optional<std::string_view> BiMap::FindValue(std::string_view key) const noexcept {
	return impl_ptr->FindValue(key);
}

std::optional<std::string_view> BiMap::FindKey(std::string_view value) const noexcept {
	return impl_ptr->FindKey(value);
}
