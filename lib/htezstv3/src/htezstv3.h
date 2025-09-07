/**
 * @file htezstv3.h
 * @brief Easy Settings Plugin for HelTec WiFi LoRa 32 V3 Board
 *
 * Description:
 *
 * This library provides a simple web interface for configuring board settings including
 * Serial, Display, LoRa, and WiFi parameters through a responsive HTML interface.
 *
 * Configuration:
 *
 * The library automatically enables WiFi and creates a web server. Access the settings
 * page at http://{board_ip}/settings to configure all board peripherals.
 *
 * Depends On:
 * - htwlv3
 *
 * @author @joaomrsouza (João Marcos Rocha Souza)
 * https://github.com/joaomrsouza
 */

#ifndef HTEZSTV3_H
#define HTEZSTV3_H

#include "htwlv3.h"

/**
 * @namespace EZSettings
 * @brief Easy Settings namespace for HelTec WiFi LoRa 32 V3 Board
 *
 * This namespace provides functions to create a web-based settings interface
 * for configuring board peripherals through a user-friendly HTML interface.
 */
namespace EZSettings
{
  /**
   * @brief Initialize the EZSettings web interface
   *
   * This function sets up the web server routes and enables the settings interface.
   * It automatically enables WiFi and server if not already enabled.
   *
   * @note The settings page will be available at /settings
   * @note Requires the Board instance to be initialized first
   *
   * @warning Call Board.begin() before calling this function
   */
  void begin();

  /**
   * @namespace detail
   * @brief Internal implementation details for EZSettings
   *
   * This namespace contains internal functions used by the EZSettings interface.
   * These functions should not be called directly by user code.
   */
  namespace detail
  {
    /**
     * @brief Handle HTTP GET requests to /settings
     *
     * Generates and serves the main settings page with current configuration values.
     */
    void handleRoot();

    /**
     * @brief Handle HTTP POST requests to /settings/save
     *
     * Processes form data and updates board configuration accordingly.
     */
    void handleSave();

    /**
     * @brief Get the base HTML template
     *
     * @return String Base HTML template with CSS styling
     */
    String getBaseTemplate();

    /**
     * @brief Get the settings form template
     *
     * @return String HTML form template for settings configuration
     */
    String getSettingsTemplate();

    /**
     * @brief Get the save confirmation template
     *
     * @return String HTML template for save confirmation page
     */
    String getSaveTemplate();

    /**
     * @brief Replace all occurrences of a substring in a string
     *
     * @param str The source string
     * @param find The substring to find
     * @param replace The replacement string
     * @return String The string with all occurrences replaced
     */
    String replaceAll(String str, String find, String replace);
  }
}

#endif // HTEZSTV3_H