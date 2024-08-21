/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utility.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: janraub <janraub@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/20 10:43:15 by janraub           #+#    #+#             */
/*   Updated: 2024/08/20 18:40:33 by janraub          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utility.hpp"

std::string    Utility::trimCommentsAndWhitespaces(std::string & line)
{
    size_t pos = line.find("#");
    if (pos != std::string::npos)
        line = line.substr(0, pos);
    pos = line.find_first_not_of(" \t");
    if (pos != std::string::npos)
        line = line.substr(pos);
    pos = line.find_last_not_of(" \t");
    if (pos != std::string::npos)
        line = line.substr(0, pos + 1);
    return line;
}