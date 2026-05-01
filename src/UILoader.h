#include <string>
#include <vector>
#include <unordered_map>

class UILoader {
public:
    struct TextureInfo {
        float x = 0.0f;
        float xRatio = 0.0f;
        float y = 0.0f;
        float yRatio = 0.0f;
        float width = 0.0f;
        float widthRatio = 0.0f;
        float height = 0.0f;
        float heightRatio = 0.0f;
    };
    struct TextInfo {
        float x = 0.0f;
        float xRatio = 0.0f;
        float y = 0.0f;
        float yRatio = 0.0f;
        float scale = 0.0f;
        float scaleRatio = 0.0f;
        std::vector<std::string> texts;
    };
    UILoader(class Game* game);
    void Initialize();
    void Load(const std::string& path);
    
    const TextureInfo* GetTextureInfo(const std::string& screenName, const std::string& id) const {
        std::string mapId = screenName + "." + id;
        auto it = mTextureInfo.find(mapId);
        return (it != mTextureInfo.end()) ? &it->second : nullptr;
    }

    const TextInfo* GetTextInfo(const std::string& screenName, const std::string& id) const {
        std::string mapId = screenName + "." + id;
        auto it = mTextInfo.find(mapId);
        return (it != mTextInfo.end()) ? &it->second : nullptr;
    }

private:
    Game* GetGame() const { return mGame; }

private:
    Game* mGame;
    std::unordered_map<std::string, TextureInfo> mTextureInfo;
    std::unordered_map<std::string, TextInfo> mTextInfo;
};